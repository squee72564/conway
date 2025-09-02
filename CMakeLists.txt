cmake_minimum_required(VERSION 3.12)

project(conway VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(conway "src/main.cpp")

find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)
target_link_libraries(conway PRIVATE SFML::Network SFML::Graphics SFML::Window SFML::Audio SFML::System)

