#include "com_jetbeep_Library.h"
#include "../../lib/libjetbeep.hpp"

using namespace JetBeep;

JNIEXPORT jstring JNICALL Java_com_jetbeep_Library_getNativeVersion(JNIEnv* env, jclass) {
  return env->NewStringUTF(Version::currentVersion().c_str());
}