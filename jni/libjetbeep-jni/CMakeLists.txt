find_package(JNI)

if (NOT JNI_FOUND)
  message("JNI libraries not found! Skipping jni build")
  return()
endif()

file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/../libjetbeep-jni-java/src/main/java/com/jetbeep/Library.java"
"package com.jetbeep;

import java.lang.RuntimeException;

public class Library {
  static {
    Library.loadAndCheckVersion();
  }

  private Library() {

  }

  public static void loadAndCheckVersion() {
    System.loadLibrary(\"jetbeep-jni\");
    if (!Library.version().equals(Library.getNativeVersion())) {
      throw new RuntimeException(\"java(\" + Library.version() +\") and native(\"+Library.getNativeVersion()+\") version mismatch\");
    }
  }
  public static final String version() {
    return \"${PROJECT_VERSION}\";
  } 
  public static native String getNativeVersion();
}"
)

add_custom_target(GenerateJavaHeaders COMMAND   
  javac -h . 
  ../libjetbeep-jni-java/src/main/java/com/jetbeep/AutoDevice.java 
  ../libjetbeep-jni-java/src/main/java/com/jetbeep/Barcode.java 
  ../libjetbeep-jni-java/src/main/java/com/jetbeep/Logger.java 
  ../libjetbeep-jni-java/src/main/java/com/jetbeep/Barcode.java 
  ../libjetbeep-jni-java/src/main/java/com/jetbeep/EasyPayBackend.java
  ../libjetbeep-jni-java/src/main/java/com/jetbeep/Library.java
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(jetbeep-jni SHARED ${JETBEEP_INCLUDES} $<TARGET_OBJECTS:jetbeep_obj> 
  AutoDevice.cpp Logger.cpp EasyPayBackend.cpp Library.cpp jni-utils.cpp)

set_target_properties(jetbeep-jni PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION}
)

add_dependencies(jetbeep-jni GenerateJavaHeaders)

target_include_directories(jetbeep-jni PUBLIC ${JNI_INCLUDE_DIRS})

# IMPORTANT: $<0:> means that no additional Debug\Release folder be created
set_target_properties(jetbeep-jni PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}$<0:>"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}$<0:>"
  )

if (APPLE)
  target_link_libraries(jetbeep-jni "-framework IOKit" "-framework CoreFoundation" "-framework Foundation")
elseif(UNIX AND NOT APPLE) #linux
  find_package(OpenSSL REQUIRED)
  find_package(CURL REQUIRED)
  add_definitions(-DCURL_STATICLIB)
  message(STATUS "Using CURL_LIBRARIES: ${CURL_LIBRARIES}.")
  target_include_directories(jetbeep-jni PUBLIC ${CURL_INCLUDE_DIRS})
  target_link_libraries(jetbeep-jni "-lpthread" "-ludev" ${CURL_LIBRARIES} -static-libgcc -static-libstdc++)
elseif(WIN32)
  target_link_libraries(jetbeep-jni "wsock32" "ws2_32" "winhttp")
endif()