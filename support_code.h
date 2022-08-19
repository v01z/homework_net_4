//Common pieces of code that are used both in server and client

#ifndef HOMEWORK_4_SUPPORT_CODE_H
#define HOMEWORK_4_SUPPORT_CODE_H

#include <iostream>
#include <boost/asio.hpp>
#include <fstream>

//--------------------------------------------------------------------------------

const uint16_t PORT{ 51511 };

//--------------------------------------------------------------------------------

bool isFileRequest(const std::string&);

//--------------------------------------------------------------------------------

std::tuple<bool, size_t, bool, size_t> getLimitsIfGiven(const std::string&);

//--------------------------------------------------------------------------------

struct FileDescr{
    std::string file_name;
    std::vector<char> mem_buff;
    std::pair<size_t, size_t> limits;

    explicit FileDescr(const std::string&, const std::vector<char>&, const std::pair
        <size_t, size_t>);
};

//--------------------------------------------------------------------------------

#endif //HOMEWORK_4_SUPPORT_CODE_H
