add_executable(autodevice main.cpp cmd.cpp)
target_include_directories(autodevice PUBLIC ${BOOST_SOURCE_DIR})

target_link_libraries(autodevice $<TARGET_OBJECTS:jetbeep_obj>)

if (APPLE) 
    find_package(OpenSSL REQUIRED)
    target_include_directories(autodevice PUBLIC ${OPENSSL_INCLUDE_DIR})
	target_link_libraries(autodevice "-framework IOKit" "-framework CoreFoundation" ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})
elseif(UNIX AND NOT APPLE) #linux
    find_package(OpenSSL REQUIRED)
    find_package(CURL REQUIRED)
    add_definitions(-DCURL_STATICLIB)
    message(STATUS "Using CURL_LIBRARIES: ${CURL_LIBRARIES}.")
    target_include_directories(autodevice PUBLIC ${CURL_INCLUDE_DIRS})
    target_link_libraries(autodevice "-lpthread" "-ludev" ${CURL_LIBRARIES})
elseif(WIN32)
    target_link_libraries(autodevice "wsock32" "ws2_32" "winhttp")
endif()