#include "support_code.h"
#include <algorithm>
#include "client.h"


//--------------------------------------------------------------------------------

Client::Client(boost::asio::io_context &io_context, const std::string &host,
               const std::string &port):
    resolver_{ io_context },
    socket_{ io_context },
    file_{}, request_{}, response_{},
    host_{ host },
    ec_{}
{
    std::cout << "Trying to resolve remote address..\n";
    const auto endpoints = resolver_.resolve(host_, port, ec_);
    errorHandler("resolve()");

     const auto connected_endpoint =
            boost::asio::connect(socket_, endpoints, ec_);
    errorHandler("connect()");

   std::cout << "Connected successfully to " << connected_endpoint << std::endl;
}

//--------------------------------------------------------------------------------

void Client::errorHandler(const std::string& err_message) {
    if(ec_)
       throw std::runtime_error(err_message + " failed. Error code: " + ec_.what());
}

//--------------------------------------------------------------------------------

void Client::run() {
    requestInput();

    while (request_ != "quit")
    {
        sendRequest();

        if (request_ == "exit") {
            std::cout << "Exit command has been sent to " << host_ << ".\n";
            return;
        }

       if(isFileRequest(request_))
       {
           std::string str_buffer{std::string(request_, 9)}; //9 == strlen("get_file:")

           file_ = std::move(std::unique_ptr<FileDescr>(new FileDescr{
                   {str_buffer.substr(str_buffer.find_last_of("/\\") + 1)},
                   {},
                   {}}));

           std::cout << file_->file_name << " has been requested." << std::endl;


           //Protocol order: Getting size of a file

           std::cout << "Getting size of file " << file_->file_name << " from remote host..\n";

           getStrResponse("#");

           if (!parseFileSizeFromStr(str_buffer))
           {
               requestInput();
               continue;
           }

           str_buffer = {};

           std::cout << "File size is " << file_->limits.second << std::endl;

           //Protocol order: Sending starting and ending positions of a file we want to get

           setFileLimitsRange();

           std::cout << "Sending range of file's buffer: " << file_->limits.first <<
                " - " << file_->limits.second << std::endl;

           request_ = std::to_string(file_->limits.first) + "#" +
                   std::to_string(file_->limits.second) + "#";

           sendRequest();

           //Protocol order: Getting file (or part of file)

           getFile();

       }
       else //Wir wollen nur plaudern
       {
        std::cout << "Getting message_ from remote host (" << host_ << ")..\n";

        response_ = {};

        boost::asio::read_until(socket_, boost::asio::dynamic_buffer(response_), "\0", ec_);
        if (ec_ && ec_.value() != 104)
        {
            std::cerr << "read() failed. Error code: " << ec_ << std::endl;
            return;
        }
        std::cout << "Got from server:\n" << response_ << std::endl;
       }
       requestInput();
    } //while() not "quit"

    std::cout << "Bye!\n";
}


//--------------------------------------------------------------------------------

void Client::requestInput() {
    request_ = {};
    std::cout << "\n**************************\n";
    std::cout << "Enter a message_/command you want to sent:\n->";

    std::getline(std::cin, request_);
    if (request_.empty())
        request_ = "quit";
}

//--------------------------------------------------------------------------------

void Client::sendRequest() {
    if(request_.empty())
        return;

    std::cout << "Sending request: " << request_ << std::endl;
    boost::asio::write(socket_, boost::asio::buffer(request_), ec_);
    errorHandler("write()");
}
//--------------------------------------------------------------------------------

void Client::getStrResponse(const std::string& delimeter) {
    response_ = {};
    boost::asio::read_until
        (socket_, boost::asio::dynamic_buffer(response_), delimeter, ec_);

    errorHandler("read_until()");
}

//--------------------------------------------------------------------------------

const size_t Client::getSizeFromUserInput() {
    size_t ret_val;
    std::string line{};
    std::cout << "Numbers only:\n->";
    while (std::getline(std::cin, line)) {
        std::stringstream ss(line);
        if (ss >> ret_val && ret_val <= SIZE_MAX) {
            if (ss.eof()) {   // Success
                break;
            }
        }
        std::cout << "Error!" << std::endl;
        std::cout << "Enter numbers only:\n->";
    }
    return ret_val;
}

//--------------------------------------------------------------------------------

void Client::setFileLimitsRange(){
    std::cout << "\n**************************\n";

    size_t file_size { file_->limits.second };

    do {
        std::cout << "Enter starting point you want to get file from (0 - "
                  << file_->limits.second << "). ";
        file_->limits.first = getSizeFromUserInput();
    } while (file_->limits.first >= file_->limits.second);

    do {
        std::cout << "Now enter ending point of file buffer ("
                  << file_->limits.first + 1 << " - " << file_->limits.second << ").\n";

        file_->limits.second = getSizeFromUserInput();
    } while (file_->limits.second <= file_->limits.first && file_->limits.second > file_size);
}

//--------------------------------------------------------------------------------

void Client::getFile(){
        std::cout << "Getting file " << file_->file_name << " from remote host..\n";

        std::ofstream file_stream
            (file_->file_name, std::ios::out | std::ofstream::binary);

        boost::asio::read(socket_,
            boost::asio::dynamic_buffer(
                file_->mem_buff, file_->limits.second - file_->limits.first), ec_);

        errorHandler("getFile()");

        std::copy(file_->mem_buff.begin(), file_->mem_buff.end(),
                      std::ostream_iterator<char>(file_stream));

        file_stream.close();
        file_ = nullptr;
}

//--------------------------------------------------------------------------------

bool Client::parseFileSizeFromStr(const std::string &full_file_path){

    assert(!file_->file_name.empty());

    size_t pos  = response_.find_first_of("#");

    if (pos == std::string::npos)
        return false;
    else {
        //9 == strlen("get_file:"), 1 == '#'
        if((full_file_path.length() + 9 + 1) == response_.length()) {
            if (std::string(response_, 9, full_file_path.length())
                == full_file_path) {
                std::cout << "File " << file_->file_name << " doesnt exists or "
                          << "cannot be opened.\n";
                return false;
            }
        }

        int i = sscanf(std::string(response_.c_str(), pos).c_str(),
               "%zu", &file_->limits.second);
        if (i == 0 || i == EOF)
            throw std::runtime_error("sscanf() failed");
    }
    return true;
}
//--------------------------------------------------------------------------------

int main(int argc, char **argv) {

    std::string hostname{"localhost"};
    if (argc == 2)
        hostname = argv[1];

    boost::asio::io_context io_context;
    try {

        Client client{io_context, hostname, std::to_string(PORT)};
        client.run();
    }
    catch(std::exception &excp) {
        std::cerr << "Error: " << excp.what() << std::endl;
    }

    return 0;
}