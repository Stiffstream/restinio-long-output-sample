cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(nodejs/http_parser)
add_subdirectory(fmt)
add_subdirectory(restinio)

add_subdirectory(long_output)

