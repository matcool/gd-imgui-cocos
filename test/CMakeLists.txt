cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(test-mod)

add_library(test-mod SHARED main.cpp)

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

add_subdirectory(../ ${CMAKE_CURRENT_BINARY_DIR}/imgui-cocos)

target_link_libraries(test-mod imgui-cocos)

setup_geode_mod(test-mod)
