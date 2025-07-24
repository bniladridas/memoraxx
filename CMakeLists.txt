cmake_minimum_required(VERSION 3.10)
project(LlamaTerminalApp)

set(CMAKE_CXX_STANDARD 11)

add_executable(LlamaTerminalApp main.cpp)

find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(LlamaTerminalApp PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)
