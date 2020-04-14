cmake_minimum_required(VERSION 3.10)

set(PRJ long_output_epr)

project(${PRJ} CXX)

add_executable(${PRJ} main.cpp)
target_link_libraries(${PRJ} PRIVATE restinio::restinio)

if (UNIX)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads)
	if (Threads_FOUND)
		target_link_libraries(${PRJ} PRIVATE Threads::Threads)
	endif ()
endif ()

if (WIN32)
        target_link_libraries(${PRJ} PRIVATE wsock32 ws2_32)
endif ()

install(
	TARGETS ${PRJ}
	RUNTIME DESTINATION bin
)

