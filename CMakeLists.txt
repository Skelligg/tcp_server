
cmake_minimum_required(VERSION 3.16)
project(tcp_project VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(tcp_core
    src/client_connection.cpp
    src/protocol_handler.cpp
    src/tcp_server.cpp
    src/tcp_client.cpp
    src/utils.cpp
)

target_include_directories(tcp_core
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
  target_compile_options(tcp_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(tcp_core PRIVATE -Wall -Wextra -pedantic -Wshadow)
endif()

add_executable(tcp_server
    src/server_main.cpp
)

target_link_libraries(tcp_server
    PRIVATE tcp_core
)

add_executable(tcp_client
    src/client_main.cpp
)

target_link_libraries(tcp_client
    PRIVATE tcp_core
)

option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

