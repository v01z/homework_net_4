#ifndef PRE_HOMEWORK_4_SERVER_H
#define PRE_HOMEWORK_4_SERVER_H

#include "support_code.h"
#include <thread>


//--------------------------------------------------------------------------------

class Session : public std::enable_shared_from_this<Session> {
private:
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    std::string message_;
    std::unique_ptr<FileDescr> file_;


    void read();
    void sendMessage();
    void sendFileSize();
    void sendFile();
    bool isFilePathValid(const std::string&);


public:
    explicit Session(std::shared_ptr<boost::asio::ip::tcp::socket>);

    friend void serve(std::shared_ptr<boost::asio::io_context> io_context,
            std::shared_ptr<boost::asio::ip::tcp::socket>);
};

//--------------------------------------------------------------------------------

#endif //PRE_HOMEWORK_4_SERVER_H
