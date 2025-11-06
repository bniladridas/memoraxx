cmake_minimum_required(VERSION 3.10)
project(memoraxx LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find CURL (required)
find_package(CURL REQUIRED)

# Find nlohmann_json (required)
find_package(nlohmann_json 3.10 REQUIRED)

# Add executable
add_executable(memoraxx src/main.cpp)

# Link libraries
target_link_libraries(memoraxx PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)