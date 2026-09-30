cmake_minimum_required(VERSION 3.20)

project(fps_arena_survival LANGUAGES CXX)

find_package(raylib CONFIG REQUIRED)

add_executable(fps_arena main.cpp src/game_rules.cpp)
target_compile_features(fps_arena PRIVATE cxx_std_17)
target_compile_options(fps_arena PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(fps_arena PRIVATE src)
target_link_libraries(fps_arena PRIVATE raylib)
