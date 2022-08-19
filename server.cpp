#include "server.h"

//--------------------------------------------------------------------------------

Session::Session(std::shared_ptr<boost::asio::ip::tcp::socket> socket) :
        socket_{ socket },
        message_{},
        file_{} {}

//--------------------------------------------------------------------------------

void Session::read() {
    boost::asio::async_read_until(*socket_,
        boost::asio::dynamic_buffer(message_), "\0",
            [self = shared_from_this()]
                (boost::system::error_code ec, std::size_t length) {

        if (ec || self->message_ == "\n") return;

        if (self->message_ == "exit") {
        std::cout << "\nGot exit command from " << self->socket_->remote_endpoint();
        std::cout << std::endl << "Bye.." << std::endl;

        self->socket_->close();
        exit(0);
        }

        std::cout << "\nGot from client " << self->socket_->remote_endpoint()
            << ":" << std::endl;
        std::cout << self->message_ << std::endl;

         if(isFileRequest(self->message_)){
             if(self->isFilePathValid(std::string(self->message_,9)))
             {
                self->sendFileSize();
             }
             else
             {
                 self->sendMessage();
             }

        } else {
            std::tuple<bool, size_t, bool, size_t>
                    is_got_limits{};
            if (self->file_)
                is_got_limits = getLimitsIfGiven(self->message_);

            if (std::get<0>(is_got_limits)) { //got starting position
                self->file_->limits.first = std::get<1>(is_got_limits);

                assert(self->file_->limits.first < self->file_->limits.second);

                std::cout << "\nGot starting reading position: "
                          << self->file_->limits.first << std::endl;

                if (std::get<2>(is_got_limits)) //got ending position
                    self->file_->limits.second = std::get<3>(is_got_limits);

                std::cout << "\nSending file..\n";
                self->sendFile();
            }
            else //Client just wants to chatting
            {
                self->message_ = "I've got from u: " + self->message_;
                self->sendMessage();
            }
        }
    });
}

//--------------------------------------------------------------------------------

void Session::sendMessage() {
    std::cout << "\nSending to client " << socket_->remote_endpoint()
              << ":" << std::endl;
    std::cout << message_ << std::endl;

    boost::asio::async_write(*socket_, boost::asio::buffer(message_),
    [self = shared_from_this()]
            (boost::system::error_code ec, std::size_t length) {
                     if (ec) return;
                     self->message_.clear();
                     self->read();
                 });
}

//--------------------------------------------------------------------------------

void Session::sendFileSize() {

    std::cout << "\nSending size of file " << file_->file_name
              << '(' << file_->limits.second << " bytes)" << " to client "
              << socket_->remote_endpoint() << std::endl;

    std::string size_buff{ std::to_string(file_->limits.second) + '#' };
    std::cout << size_buff << std::endl; //debug

    boost::asio::async_write(*socket_, boost::asio::buffer(size_buff),
    [self = shared_from_this()]
            (boost::system::error_code ec, std::size_t length) {
                     if (ec) return;
                     self->message_.clear();
                     self->read();
                 });

}

//--------------------------------------------------------------------------------

void Session::sendFile() {

    assert(file_ != nullptr);

    boost::asio::async_write(*socket_,
        boost::asio::buffer((file_->mem_buff).data() + file_->limits.first,
            file_->limits.second - file_->limits.first),
                [self = shared_from_this()]
                (boost::system::error_code ec, std::size_t length) {
                     if (ec) return;
                     self->message_.clear();
                     std::cout << "\nFile " << self->file_->file_name << " has been sent.\n";
                     self->file_ = nullptr;
                     self->read();
                 });
}

//--------------------------------------------------------------------------------

bool Session::isFilePathValid(const std::string &file_name) {

    std::ifstream file_stream;
    file_stream.open(file_name, std::ios::in);
    if(!file_stream)
    {
        // '#' at the end of string will help to recognize it
        // as a service message (get_file:file_name + #)
       std::cout << "\nFile " << file_name << " doesnt exists or cannot be opened.\n";
       message_.append("#");
       return false;
    }

     file_ = nullptr;

     file_stream.seekg(0, std::ios::end);

    size_t file_len = file_stream.tellg();
    std::vector<char> vec_buff(file_len);

    file_stream.seekg(0, std::ios::beg);

    if (file_len)
        file_stream.read(&vec_buff[0], file_len);
    file_stream.close();

    file_ = move(std::unique_ptr<FileDescr>(new FileDescr{
            file_name, vec_buff, { 0, file_len } } ));

    return true;
}
//--------------------------------------------------------------------------------

void serve(std::shared_ptr<boost::asio::io_context> io_context,
    std::shared_ptr<boost::asio::ip::tcp::socket> sock) {

        std::cout << "\n Socket moved to a new thread:\n ID: "
        << std::this_thread::get_id() << std::endl;
    std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";

    boost::system::error_code ec;

    auto session = std::make_shared<Session>(sock);
    session->read();
    io_context->run();
}

//--------------------------------------------------------------------------------

int main(int argc, char** argv) {
    try {
        std::shared_ptr<boost::asio::io_context> io_context =
                std::make_shared<boost::asio::io_context>();

        boost::asio::ip::tcp::acceptor acceptor  {*io_context,
    boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
    PORT) };

        std::cout << "\n####################################\n ";
        std::cout << argv[0] << " started at: " << acceptor.local_endpoint() << std::endl;
        std::cout << "\n Main thread ID: " << std::this_thread::get_id() << std::endl;
        std::cout << "####################################\n";


        while(true)
        {
            std::shared_ptr<boost::asio::ip::tcp::socket> socket =
                    std::make_shared<boost::asio::ip::tcp::socket>(*io_context.get());

            acceptor.accept(*socket.get());

            std::cout << "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n ";
            std::cout << "New connection from: " << socket->remote_endpoint() << std::endl;

            std::thread thr(serve, io_context, socket);
            thr.detach();
        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

}

//--------------------------------------------------------------------------------
