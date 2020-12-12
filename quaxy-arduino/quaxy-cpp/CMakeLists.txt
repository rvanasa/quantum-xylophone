cmake_minimum_required(VERSION 3.15)
project(quaxy_cpp)

set(CMAKE_CXX_STANDARD 11)

file(GLOB quaxy_cpp_SRC
    "*.cpp"
    "*.hpp"
)

add_executable(quaxy_cpp ${quaxy_cpp_SRC})
