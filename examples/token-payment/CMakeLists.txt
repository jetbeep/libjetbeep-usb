add_executable(token_payment_example main.cpp)

target_include_directories(token_payment_example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(token_payment_example $<TARGET_OBJECTS:jetbeep_obj>)

if (APPLE) 
	target_link_libraries(token_payment_example "-framework IOKit" "-framework CoreFoundation" "-framework Foundation")
elseif(UNIX AND NOT APPLE) #linux
    find_package(OpenSSL REQUIRED)
    find_package(CURL REQUIRED)
    add_definitions(-DCURL_STATICLIB)
    message(STATUS "Using CURL_LIBRARIES: ${CURL_LIBRARIES}.")
    target_include_directories(token_payment_example PUBLIC ${CURL_INCLUDE_DIRS})
    target_link_libraries(token_payment_example "-lpthread" "-ludev" ${CURL_LIBRARIES})
elseif(WIN32)
    target_link_libraries(token_payment_example "wsock32" "ws2_32" "winhttp")
endif()