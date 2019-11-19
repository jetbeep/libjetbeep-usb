add_executable(toke_payment_test main.cpp)

target_include_directories(toke_payment_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(toke_payment_test $<TARGET_OBJECTS:jetbeep_obj>)

find_package(OpenSSL REQUIRED)
target_link_libraries(toke_payment_test "OpenSSL::SSL")

if (APPLE)
	target_link_libraries(toke_payment_test "-framework IOKit" "-framework CoreFoundation")
elseif(UNIX AND NOT APPLE) #linux
	target_link_libraries(toke_payment_test "-lpthread" "-ludev")
elseif(WIN32)
	target_link_libraries(toke_payment_test "-lwsock32")
endif()