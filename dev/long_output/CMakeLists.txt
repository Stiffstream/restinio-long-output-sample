cmake_minimum_required(VERSION 3.10)

set(PRJ long_output)

project(${PRJ})

add_executable(${PRJ} main.cpp)
target_link_libraries(${PRJ} restinio::restinio)

install(
	TARGETS ${PRJ}
	RUNTIME DESTINATION bin
)

