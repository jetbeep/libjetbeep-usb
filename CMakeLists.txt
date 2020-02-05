cmake_minimum_required (VERSION 3.10.2)

#uncomment to force 32bit build under gcc\g++. Also install libcurl4-openssl-dev:i386 libudev-dev:i386 on 64bit systems after dpkg --add-architecture i386
#set(CMAKE_C_FLAGS -m32)
#set(CMAKE_CXX_FLAGS -m32)

file (READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" JETBEEP_VERSION)
project (libjetbeep VERSION ${JETBEEP_VERSION})

add_definitions("-DJETBEEP_VERSION=\"${JETBEEP_VERSION}\"" 
    "-DBOOST_DATE_TIME_NO_LIB" "-DBOOST_REGEX_NO_LIB" "-DBOOST_THREAD_NO_LIB" "-DBOOST_CHRONO_NO_LIB")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOOST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/boost_1_71_0)
set(JETBEEP_LIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_subdirectory(lib)

add_subdirectory(examples/detection-and-serial)
add_subdirectory(examples/autodevice)
add_subdirectory(examples/token-payment)
add_subdirectory(jni/libjetbeep-jni)

add_subdirectory(dfu-module)
