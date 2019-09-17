cmake_minimum_required (VERSION 3.10.2)
project (libjetbeep VERSION 0.1.0)

add_definitions("-std=c++11")

add_subdirectory(lib)

enable_testing()
add_subdirectory(examples)
