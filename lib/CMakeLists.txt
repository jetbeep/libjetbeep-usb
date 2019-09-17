
include(GNUInstallDirs)
file(GLOB JETBEEP_SOURCES *.cpp *.c)
file(GLOB JETBEEP_INCLUDES *.h)
set(BOOST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boost_1_71_0)

add_library(jetbeep_obj OBJECT ${JETBEEP_SOURCES} ${JETBEEP_INCLUDES})
target_include_directories(jetbeep_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${BOOST_SOURCE_DIR})

add_library(jetbeep SHARED ${JETBEEP_INCLUDES} $<TARGET_OBJECTS:jetbeep_obj>)
target_include_directories(jetbeep PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(jetbeep PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0.1.0
  PUBLIC_HEADER ${JETBEEP_INCLUDES}
)

add_library(jetbeep_static STATIC ${JETBEEP_INCLUDES} $<TARGET_OBJECTS:jetbeep_obj>)
target_include_directories(jetbeep_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(jetbeep_static PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0.1.0
  PUBLIC_HEADER ${JETBEEP_INCLUDES}
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
