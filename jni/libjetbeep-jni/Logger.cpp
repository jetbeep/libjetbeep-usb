#include "com_jetbeep_Logger.h"
#include "jni-utils.hpp"

using namespace JetBeep;
using namespace std;

JNIEXPORT void JNICALL Java_com_jetbeep_Logger_initJvm(JNIEnv* env, jclass className) {
  JniUtils::storeJvm(env);
}

JNIEXPORT void JNICALL Java_com_jetbeep_Logger_setCoutEnabled(JNIEnv* env, jclass className, jboolean enabled) {
  Logger::coutEnabled = (bool)enabled;
}

JNIEXPORT void JNICALL Java_com_jetbeep_Logger_setCerrEnabled(JNIEnv* env, jclass className, jboolean enabled) {
  Logger::cerrEnabled = (bool)enabled;
}

JNIEXPORT jint JNICALL Java_com_jetbeep_Logger_nativeGetLogLevel(JNIEnv* env, jclass className) {
  return (jint)Logger::level;
}

JNIEXPORT void JNICALL Java_com_jetbeep_Logger_nativeSetLogLevel(JNIEnv* env, jclass className, jint jlogLevel) {
  Logger::level = (LoggerLevel)jlogLevel;
}
