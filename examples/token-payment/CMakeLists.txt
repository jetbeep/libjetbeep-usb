add_executable(token_payment_example main.cpp)

target_include_directories(token_payment_example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(token_payment_example $<TARGET_OBJECTS:jetbeep_obj>)

find_package(OpenSSL REQUIRED)
target_link_libraries(token_payment_example ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})

if (APPLE)
	target_link_libraries(token_payment_example "-framework IOKit" "-framework CoreFoundation")
elseif(UNIX AND NOT APPLE) #linux
	target_link_libraries(token_payment_example "-lpthread" "-ludev")
elseif(WIN32)
	target_link_libraries(token_payment_example "-lwsock32")
endif()