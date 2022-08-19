cmake_minimum_required(VERSION 3.22)
project(homework_net_4)

set(CMAKE_CXX_STANDARD 17)

add_executable(server server.cpp support_code.cpp)

add_executable(client client.cpp support_code.cpp)


