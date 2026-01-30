cmake_minimum_required(VERSION 3.12)

project(conway VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(conway "src/main.cpp")

find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)
target_link_libraries(conway PRIVATE SFML::Network SFML::Graphics SFML::Window SFML::Audio SFML::System)

add_executable(bench_hash "bench/bench_hash.cpp")
target_include_directories(bench_hash PRIVATE "src")
target_link_libraries(bench_hash PRIVATE SFML::System)
