#include "jni-utils.hpp"

using namespace std;
using namespace JetBeep;

Logger JniUtils::log = Logger("jni-utils");

void JniUtils::throwIllegalStateException(JNIEnv* env, const std::string& message) {
  jclass exClass;
  const char* className = "java/lang/IllegalStateException";

  exClass = env->FindClass(className);
  if (exClass == NULL) {
    log.e() << "unable to find illegalstate exception" << Logger::endl;
    return;
  }

  if (env->ThrowNew(exClass, message.c_str()) != 0) {
    log.e() << "unable to throwNew" << Logger::endl;
    return;
  }
}

void JniUtils::throwNullPointerException(JNIEnv* env, const std::string& message) {
  jclass exClass;
  const char* className = "java/lang/NullPointerException";

  exClass = env->FindClass(className);
  if (exClass == NULL) {
    log.e() << "unable to find NullPointerException exception" << Logger::endl;
    return;
  }

  if (env->ThrowNew(exClass, message.c_str()) != 0) {
    log.e() << "unable to throwNew" << Logger::endl;
    return;
  }  
}
bool JniUtils::getAutoDevicePointer(JNIEnv *env, jlong ptr, AutoDevice **autoDevice) {
  if (0 == ptr) {
    JniUtils::throwNullPointerException(env, "AutoDevice pointer is null");
    *autoDevice = nullptr;
    return false;
  }
  *autoDevice = static_cast<AutoDevice*>((void*)ptr);
  return true;
}

std::string JniUtils::getString(JNIEnv *env, jstring string) {
  auto cstr = env->GetStringUTFChars(string, nullptr);
  std::string returnString = std::string(cstr == nullptr ? "" : cstr);
  env->ReleaseStringUTFChars(string, cstr);
  return returnString;
}