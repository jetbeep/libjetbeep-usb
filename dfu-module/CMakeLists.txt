aux_source_directory(${BOOST_SOURCE_DIR}/libs/filesystem/src BOOST_FILESYSTEM_SOURCES) 

add_executable(dfu_module ${BOOST_FILESYSTEM_SOURCES} 
            dfu_main.cpp 
            serial_device.cpp
            packages_search.cpp
            crc32.c 
            delay_connect.c
            dfu.c 
            dfu_serial.c 
            slip_enc.c 
            uart_drv.cpp 
            uart_slip.c 
            zip.c 
            logging.c 
            jsmn.c 
)

#set_source_files_properties( uart_drv.cpp dfu.c uart_slip.c dfu_serial.c PROPERTIES LANGUAGE CXX )

target_include_directories(dfu_module PUBLIC ${BOOST_SOURCE_DIR} ${BOOST_SOURCE_DIR}/lib/filesystem/)

target_link_libraries(dfu_module $<TARGET_OBJECTS:jetbeep_obj>)

if (APPLE) 
    find_package(OpenSSL REQUIRED)
    target_include_directories(dfu_module PUBLIC ${OPENSSL_INCLUDE_DIR})
	target_link_libraries(dfu_module "-framework IOKit" "-framework CoreFoundation" ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})
elseif(UNIX AND NOT APPLE) #linux
    find_package(OpenSSL REQUIRED)
    find_package(CURL REQUIRED)
    add_definitions(-DCURL_STATICLIB)
    message(STATUS "Using CURL_LIBRARIES: ${CURL_LIBRARIES}.")
    target_include_directories(dfu_module PUBLIC ${CURL_INCLUDE_DIRS})
    target_link_libraries(dfu_module "-lpthread" "-ludev" ${CURL_LIBRARIES})
elseif(WIN32)
    target_link_libraries(dfu_module "wsock32" "ws2_32" "winhttp")
endif()

#install(TARGETS dfu_module RUNTIME DESTINATION bin)