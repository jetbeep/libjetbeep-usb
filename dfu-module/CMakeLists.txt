add_executable(dfu_module dfu_main.cpp crc32.c delay_connect.c dfu.c dfu_serial.c slip_enc.c uart_drv.c uart_slip.c zip.c)

target_include_directories(dfu_module PUBLIC ${BOOST_SOURCE_DIR})

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

install(TARGETS dfu_module RUNTIME DESTINATION bin)