add_executable(jetbeep_usb_test main.cpp cmd.cpp)
target_include_directories(jetbeep_usb_test PUBLIC ${BOOST_SOURCE_DIR})

target_link_libraries(jetbeep_usb_test $<TARGET_OBJECTS:jetbeep_obj>)

if (APPLE)
	target_link_libraries(jetbeep_usb_test "-framework IOKit" "-framework CoreFoundation")
endif()