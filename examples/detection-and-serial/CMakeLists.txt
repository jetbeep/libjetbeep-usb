add_executable(detection_and_serial main.cpp cmd.cpp promise_test.cpp)
target_include_directories(detection_and_serial PUBLIC ${BOOST_SOURCE_DIR})

add_dependencies(detection_and_serial jetbeep_obj)
target_link_libraries(detection_and_serial $<TARGET_OBJECTS:jetbeep_obj>)

if (APPLE) 
	target_link_libraries(detection_and_serial "-framework IOKit" "-framework CoreFoundation" "-framework Foundation")
elseif(UNIX AND NOT APPLE) #linux
    find_package(OpenSSL REQUIRED)
    find_package(CURL REQUIRED)
    add_definitions(-DCURL_STATICLIB)
    message(STATUS "Using CURL_LIBRARIES: ${CURL_LIBRARIES}.")
    target_include_directories(detection_and_serial PUBLIC ${CURL_INCLUDE_DIRS})
    target_link_libraries(detection_and_serial "-lpthread" "-ludev" ${CURL_LIBRARIES} -static-libgcc -static-libstdc++)
elseif(WIN32)
    target_link_libraries(detection_and_serial "wsock32" "ws2_32" "winhttp")
endif()