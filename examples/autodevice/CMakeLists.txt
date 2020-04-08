add_executable(autodevice main.cpp cmd.cpp)
target_include_directories(autodevice PUBLIC ${BOOST_SOURCE_DIR})

add_dependencies(autodevice jetbeep_obj)
target_link_libraries(autodevice $<TARGET_OBJECTS:jetbeep_obj>)

if (APPLE) 
	target_link_libraries(autodevice "-framework IOKit" "-framework CoreFoundation" "-framework Foundation")
elseif(UNIX AND NOT APPLE) #linux
    find_package(udev REQUIRED)
    find_package(OpenSSL REQUIRED)
    find_package(CURL REQUIRED)
    add_definitions(-DCURL_STATICLIB)
    message(STATUS "Using CURL_LIBRARIES: ${CURL_LIBRARIES}.")
    target_include_directories(autodevice PUBLIC ${CURL_INCLUDE_DIRS})
    target_link_libraries(autodevice "-lpthread" ${UDEV_LIBRARIES} ${CURL_LIBRARIES} -static-libgcc -static-libstdc++)
elseif(WIN32)
    target_link_libraries(autodevice "wsock32" "ws2_32" "winhttp")
endif()