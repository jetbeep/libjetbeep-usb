
include(GNUInstallDirs)
file(GLOB_RECURSE JETBEEP_SOURCES *.cpp *.c)
file(GLOB_RECURSE JETBEEP_INCLUDES *.h *.hpp)
list(FILTER JETBEEP_SOURCES EXCLUDE REGEX "boost")

set(BOOST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boost_1_71_0)

add_library(jetbeep_obj OBJECT ${JETBEEP_SOURCES} ${JETBEEP_INCLUDES})
set_property(TARGET jetbeep_obj PROPERTY POSITION_INDEPENDENT_CODE ON)

find_package(OpenSSL REQUIRED)

target_include_directories(jetbeep_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${BOOST_SOURCE_DIR} ${OPENSSL_INCLUDE_DIR})

add_library(jetbeep SHARED ${JETBEEP_INCLUDES} $<TARGET_OBJECTS:jetbeep_obj>)


target_link_libraries(jetbeep ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})

if (APPLE) 
	target_link_libraries(jetbeep "-framework IOKit" "-framework CoreFoundation")
elseif(UNIX AND NOT APPLE) #linux
    target_link_libraries(jetbeep "-lpthread" "-ludev")
elseif(WIN32)
    target_link_libraries(jetbeep "-lwsock32")
endif()

target_include_directories(jetbeep PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#set_target_properties(jetbeep PROPERTIES
#  VERSION ${PROJECT_VERSION}
#  SOVERSION 0.1.0
#  PUBLIC_HEADER ${JETBEEP_INCLUDES}
#)

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
