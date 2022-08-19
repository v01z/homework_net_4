#include "support_code.h"

#ifndef VER3_HOMEWORK4_CLIENT_H
#define VER3_HOMEWORK4_CLIENT_H

//--------------------------------------------------------------------------------

class Client{
private:
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::socket socket_;
    std::unique_ptr<FileDescr> file_;
    std::string request_, response_;
    const std::string host_;
    boost::system::error_code ec_;

    void errorHandler(const std::string&);
    void requestInput();
    void sendRequest();
    void getStrResponse(const std::string&);
    const size_t getSizeFromUserInput();
    void setFileLimitsRange();
    void sendFilePos();
    void getFile();
    bool parseFileSizeFromStr(const std::string&);

public:
    explicit Client(boost::asio::io_context&, const std::string &host, const std::string &port);

    void run();
};

//--------------------------------------------------------------------------------

#endif //VER3_HOMEWORK4_CLIENT_H
