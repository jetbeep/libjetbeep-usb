#include "com_jetbeep_JetBeep.h"

JNIEXPORT jint JNICALL Java_com_jetbeep_JetBeep_test
  (JNIEnv *, jobject, jint num) {
  return num * num;
}