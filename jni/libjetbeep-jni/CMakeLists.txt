find_package(JNI)

if (NOT JNI_FOUND)
  message("JNI libraries not found! Skipping jni build")
  return()
endif()

add_custom_target(GenerateJavaHeaders COMMAND   
  javac -h . 
  ../libjetbeep-jni-java/src/main/java/com/jetbeep/AutoDevice.java 
  ../libjetbeep-jni-java/src/main/java/com/jetbeep/Barcode.java 
  ../libjetbeep-jni-java/src/main/java/com/jetbeep/Logger.java 
  ../libjetbeep-jni-java/src/main/java/com/jetbeep/Barcode.java 
  ../libjetbeep-jni-java/src/main/java/com/jetbeep/EasyPayBackend.java
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(jetbeep-jni SHARED ${JETBEEP_INCLUDES} $<TARGET_OBJECTS:jetbeep_obj> 
  AutoDevice.cpp Logger.cpp EasyPayBackend.cpp jni-utils.cpp)
add_dependencies(jetbeep-jni GenerateJavaHeaders)

target_include_directories(jetbeep-jni PUBLIC ${JNI_INCLUDE_DIRS})

set_target_properties(jetbeep-jni PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenSSL REQUIRED)
target_link_libraries(jetbeep-jni ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})

if (APPLE) 
	target_link_libraries(jetbeep-jni "-framework IOKit" "-framework CoreFoundation")
elseif(UNIX AND NOT APPLE) #linux
  target_link_libraries(jetbeep-jni "-lpthread" "-ludev")
elseif(WIN32)
  target_link_libraries(jetbeep-jni "-lwsock32")
endif()