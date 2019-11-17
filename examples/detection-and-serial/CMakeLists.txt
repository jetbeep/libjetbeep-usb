add_executable(detection_and_serial main.cpp cmd.cpp promise_test.cpp utils.cpp)
target_include_directories(detection_and_serial PUBLIC ${BOOST_SOURCE_DIR})

target_link_libraries(detection_and_serial $<TARGET_OBJECTS:jetbeep_obj>)

if (APPLE)
	target_link_libraries(detection_and_serial "-framework IOKit" "-framework CoreFoundation")
elseif(UNIX AND NOT APPLE) #linux
	target_link_libraries(detection_and_serial "-lpthread" "-ludev")
elseif(WIN32)
	target_link_libraries(detection_and_serial "-lwsock32")
endif()