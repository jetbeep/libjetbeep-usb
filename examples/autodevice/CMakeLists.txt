add_executable(autodevice main.cpp cmd.cpp)
target_include_directories(autodevice PUBLIC ${BOOST_SOURCE_DIR})

target_link_libraries(autodevice $<TARGET_OBJECTS:jetbeep_obj>)

find_package(OpenSSL REQUIRED)
target_link_libraries(autodevice ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})

if (APPLE)
	target_link_libraries(autodevice "-framework IOKit" "-framework CoreFoundation")
elseif(UNIX AND NOT APPLE) #linux
	target_link_libraries(autodevice "-lpthread" "-ludev")
elseif(WIN32)
	target_link_libraries(autodevice "-lwsock32")
endif()