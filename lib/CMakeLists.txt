
include(GNUInstallDirs)
include("TargetArch.cmake")
file(GLOB_RECURSE JETBEEP_SOURCES *.cpp *.c *.mm)
file(GLOB_RECURSE JETBEEP_INCLUDES *.h *.hpp)
list(FILTER JETBEEP_SOURCES EXCLUDE REGEX "boost")

set(BOOST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boost_1_71_0)

add_library(jetbeep_obj OBJECT ${JETBEEP_SOURCES} ${JETBEEP_INCLUDES})
set_property(TARGET jetbeep_obj PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(jetbeep_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${BOOST_SOURCE_DIR})

add_library(jetbeep SHARED ${JETBEEP_INCLUDES} $<TARGET_OBJECTS:jetbeep_obj>)

if (APPLE) 
	target_link_libraries(jetbeep "-framework IOKit" "-framework CoreFoundation" "-framework Foundation")
elseif(UNIX AND NOT APPLE) #linux
    find_package(OpenSSL REQUIRED)
    find_package(CURL REQUIRED)
    add_definitions(-DCURL_STATICLIB)
    message(STATUS "Using CURL_LIBRARIES: ${CURL_LIBRARIES}.")
    target_include_directories(jetbeep_obj PUBLIC ${CURL_INCLUDE_DIRS})
    target_link_libraries(jetbeep "-lpthread" "-ludev" ${CURL_LIBRARIES})
elseif(WIN32)
    target_link_libraries(jetbeep "wsock32" "ws2_32" "winhttp")
    target_architecture(ARCH_NAME)
    set(DELPHI_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../delphi/$<TARGET_FILE_BASE_NAME:jetbeep>_${ARCH_NAME}$<TARGET_FILE_SUFFIX:jetbeep>")
    add_custom_command(TARGET jetbeep POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:jetbeep> ${DELPHI_OUTPUT_PATH}
    COMMENT "Copying dll to delphi folder")
endif()

target_include_directories(jetbeep PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(jetbeep PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
)

install(TARGETS jetbeep
 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
 PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
