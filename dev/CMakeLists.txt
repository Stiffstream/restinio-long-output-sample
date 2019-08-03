cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use standealone ASIO.
SET(RESTINIO_USE_BOOST_ASIO "none")
SET( RESTINIO_STAND_ALONE_ASIO_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/asio/include )
SET( RESTINIO_STAND_ALONE_ASIO_DEFINES -DASIO_STANDALONE -DASIO_HAS_STD_CHRONO -DASIO_DISABLE_STD_STRING_VIEW)

add_subdirectory(nodejs/http_parser)
add_subdirectory(fmt)
add_subdirectory(restinio)

add_subdirectory(long_output)

