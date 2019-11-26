add_executable(detection_and_serial main.cpp cmd.cpp promise_test.cpp)
target_include_directories(detection_and_serial PUBLIC ${BOOST_SOURCE_DIR})

target_link_libraries(detection_and_serial $<TARGET_OBJECTS:jetbeep_obj>)

find_package(OpenSSL REQUIRED)
target_link_libraries(detection_and_serial ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})


if (APPLE)
	target_link_libraries(detection_and_serial "-framework IOKit" "-framework CoreFoundation")
elseif(UNIX AND NOT APPLE) #linux
	target_link_libraries(detection_and_serial "-lpthread" "-ludev")
elseif(WIN32)
	target_link_libraries(jetbeep "-lwsock32" "-lws2_32")
endif()