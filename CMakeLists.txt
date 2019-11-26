cmake_minimum_required (VERSION 3.10.2)
file (READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" JETBEEP_VERSION)
project (libjetbeep VERSION ${JETBEEP_VERSION})

add_definitions("-std=c++17" "-DJETBEEP_VERSION=\"${JETBEEP_VERSION}\"")

add_subdirectory(lib)

add_subdirectory(examples/detection-and-serial)
add_subdirectory(examples/autodevice)
add_subdirectory(examples/token-payment)
add_subdirectory(jni/libjetbeep-jni)
