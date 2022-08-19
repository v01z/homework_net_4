#include "support_code.h"

//--------------------------------------------------------------------------------

bool isFileRequest(const std::string &request){
    return std::string(request, 0, 9) == "get_file:";
}

//--------------------------------------------------------------------------------

std::tuple<bool, size_t, bool, size_t> getLimitsIfGiven(const std::string& input_str){

    bool found_start{}, found_end{};
    size_t start_point{}, end_point{}, first_index{}, second_index{};

    first_index = input_str.find_first_of("#");
    if (first_index == std::string::npos){
        found_start = false;
        found_end = false;
    }
    else {
        found_start = true;

       auto lambda {[](const int i){
           if (i == 0 || i == EOF)
               throw std::runtime_error("sscanf() failed");
       }};

        int i = sscanf(std::string(input_str.c_str(), first_index).c_str(),
               "%zu", &start_point);
        lambda(i);

        second_index = input_str.find_first_of("#", first_index + 1);
        if (second_index != std::string::npos) {
            found_end = true;

        i = sscanf(std::string(input_str.c_str() + first_index + 1,
                second_index - first_index).c_str(), "%zu", &end_point);
        lambda(i);
        }
    }

    return std::tie(found_start, start_point, found_end, end_point);
}

//--------------------------------------------------------------------------------

FileDescr::FileDescr(const std::string &fn, const std::vector<char> &vec,
                     const std::pair<size_t, size_t> borders):
        file_name{fn}, mem_buff{vec}, limits{ borders }
{}

//--------------------------------------------------------------------------------