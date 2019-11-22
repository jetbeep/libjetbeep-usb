cmake_minimum_required (VERSION 3.10.2)
project (libjetbeep VERSION 0.1.0)

add_definitions("-std=c++17")

add_subdirectory(lib)

add_subdirectory(examples/detection-and-serial)
add_subdirectory(examples/autodevice)
add_subdirectory(jni/libjetbeep-jni)
