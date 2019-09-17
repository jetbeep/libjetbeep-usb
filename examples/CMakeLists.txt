add_executable(jetbeep_usb_test main.cpp)
target_include_directories(jetbeep_usb_test PUBLIC ${BOOST_SOURCE_DIR})

target_link_libraries(jetbeep_usb_test jetbeep_static)
