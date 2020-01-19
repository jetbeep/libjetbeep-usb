cmake_minimum_required (VERSION 3.10.2)
file (READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" JETBEEP_VERSION)
project (libjetbeep VERSION ${JETBEEP_VERSION})

add_definitions("-DJETBEEP_VERSION=\"${JETBEEP_VERSION}\"" 
    "-DBOOST_DATE_TIME_NO_LIB" "-DBOOST_REGEX_NO_LIB" "-DBOOST_THREAD_NO_LIB" "-DBOOST_CHRONO_NO_LIB")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)

add_subdirectory(examples/detection-and-serial)
add_subdirectory(examples/autodevice)
add_subdirectory(examples/token-payment)
add_subdirectory(jni/libjetbeep-jni)
