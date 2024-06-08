cmake_minimum_required(VERSION 3.10)
project(SrikurCHIP8 VERSION 1.0)
find_package(SDL2 REQUIRED COMPONENTS SDL2)
set(CMAKE_CXX_STANDARD 17)

add_executable(SrikurCHIP8 main.cpp cpu.cpp cpu.h)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SrikurCHIP8 PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(SrikurCHIP8 PRIVATE SDL2::SDL2)