cmake_minimum_required(VERSION 3.10)
project(Memoraxx LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find CURL (required)
find_package(CURL REQUIRED)

# Find nlohmann_json (required)
find_package(nlohmann_json 3.10 REQUIRED)

# Add executable
add_executable(Memoraxx main.cpp)

# Link libraries
target_link_libraries(Memoraxx PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)