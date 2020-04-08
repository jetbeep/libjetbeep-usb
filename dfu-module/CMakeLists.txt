aux_source_directory(${BOOST_SOURCE_DIR}/libs/filesystem/src BOOST_FILESYSTEM_SOURCES) 

add_definitions(-DWIN32_LEAN_AND_MEAN )

add_executable(dfu_module ${BOOST_FILESYSTEM_SOURCES} 
            src/dfu_main.cpp 
            src/sync_serial_device.cpp
            src/packages_search.cpp
            src/crc32.c 
            src/delay_connect.c
            src/dfu.c 
            src/dfu_serial.c 
            src/slip_enc.c 
            src/uart_drv.cpp 
            src/uart_slip.c 
            src/zip.c 
            src/logging.cpp 
            src/jsmn.c 
            src/ext_error.cpp 
)

#set_source_files_properties( uart_drv.cpp dfu.c uart_slip.c dfu_serial.c PROPERTIES LANGUAGE CXX )

target_include_directories(dfu_module PRIVATE ./include PUBLIC ${JETBEEP_LIB_SOURCE_DIR} ${BOOST_SOURCE_DIR} ${BOOST_SOURCE_DIR}/lib/filesystem/)

add_dependencies(dfu_module jetbeep_obj)

target_link_libraries(dfu_module $<TARGET_OBJECTS:jetbeep_obj>)

if (APPLE) 
	target_link_libraries(dfu_module "-framework IOKit" "-framework CoreFoundation" "-framework Foundation")
elseif(UNIX AND NOT APPLE) #linux
    find_package(udev REQUIRED)
    find_package(OpenSSL REQUIRED)
    find_package(CURL REQUIRED)
    add_definitions(-DCURL_STATICLIB)
    message(STATUS "Using CURL_LIBRARIES: ${CURL_LIBRARIES}.")
    target_include_directories(dfu_module PUBLIC ${CURL_INCLUDE_DIRS})
    target_link_libraries(dfu_module "-lpthread" ${UDEV_LIBRARIES} ${CURL_LIBRARIES})
elseif(WIN32)
    target_link_libraries(dfu_module "wsock32" "ws2_32" "winhttp")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    # disable autolinking in boost HACK to force build filesystem lib from sources
    add_definitions(-DBOOST_ALL_NO_LIB -DBOOST_ALL_DYN_LINK)
endif()

#install(TARGETS dfu_module RUNTIME DESTINATION bin)