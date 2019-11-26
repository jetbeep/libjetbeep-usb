#include "jni-utils.hpp"

using namespace std;
using namespace JetBeep;

Logger JniUtils::m_log = Logger("jni-utils");
JavaVM* JniUtils::m_jvm = nullptr;
recursive_mutex JniUtils::mutex = recursive_mutex();

void JniUtils::throwIllegalStateException(JNIEnv* env, const std::string& message) {
  jclass exClass;
  const char* className = "java/lang/IllegalStateException";

  exClass = env->FindClass(className);
  if (exClass == NULL) {
    m_log.e() << "unable to find illegalstate exception" << Logger::endl;
    return;
  }

  if (env->ThrowNew(exClass, message.c_str()) != 0) {
    m_log.e() << "unable to throwNew" << Logger::endl;
    return;
  }
}

void JniUtils::throwNullPointerException(JNIEnv* env, const std::string& message) {
  jclass exClass;
  const char* className = "java/lang/NullPointerException";

  exClass = env->FindClass(className);
  if (exClass == NULL) {
    m_log.e() << "unable to find NullPointerException exception" << Logger::endl;
    return;
  }

  if (env->ThrowNew(exClass, message.c_str()) != 0) {
    m_log.e() << "unable to throwNew" << Logger::endl;
    return;
  }
}

void JniUtils::throwIOException(JNIEnv *env, const std::string& message) {
  jclass exClass;
  const char* className = "java/io/IOException";

  exClass = env->FindClass(className);
  if (exClass == NULL) {
    m_log.e() << "unable to find IOException exception" << Logger::endl;
    return;
  }

  if (env->ThrowNew(exClass, message.c_str()) != 0) {
    m_log.e() << "unable to throwNew" << Logger::endl;
    return;
  }
}

bool JniUtils::getAutoDevicePointer(JNIEnv* env, jlong ptr, AutoDevice** autoDevice) {
  if (0 == ptr) {
    JniUtils::throwNullPointerException(env, "AutoDevice pointer is null");
    *autoDevice = nullptr;
    return false;
  }
  *autoDevice = static_cast<AutoDevice*>((void*)ptr);
  return true;
}

std::string JniUtils::getString(JNIEnv* env, jstring string) {
  auto cstr = env->GetStringUTFChars(string, nullptr);
  std::string returnString = std::string(cstr == nullptr ? "" : cstr);
  env->ReleaseStringUTFChars(string, cstr);
  return returnString;
}

void JniUtils::storeJvm(JNIEnv* env) {
  if (m_jvm != nullptr) {
    return;
  }

  env->GetJavaVM(&m_jvm);
}

JavaVM* JniUtils::getJvm() {
  return m_jvm;
}

JNIEnv* JniUtils::attachCurrentThread() {
  auto jvm = JniUtils::getJvm();
  JNIEnv* env = nullptr;
  jint errorCode = 0;

  if (jvm == nullptr) {
    m_log.e() << "attachCurrentThread: JVM is null!" << Logger::endl;
    return nullptr;
  }

  errorCode = jvm->AttachCurrentThread((void**)&env, nullptr);
  if (errorCode != JNI_OK) {
    m_log.e() << "unable to attach current thread: " << errorCode << Logger::endl;
    return nullptr;
  }

  return env;
}

void JniUtils::detachCurrentThread() {
  auto jvm = JniUtils::getJvm();
  jint errorCode = 0;

  if (jvm == nullptr) {
    m_log.e() << "detachCurrentThread: JVM is null!" << Logger::endl;
    return;
  }

  errorCode = jvm->DetachCurrentThread();
  if (errorCode != JNI_OK) {
    m_log.e() << "unable to detach current thread: " << errorCode << Logger::endl;
    return;
  }
}

jobject JniUtils::convertAutoDeviceState(JNIEnv* env, const AutoDeviceState& state) {
  string className = "com/jetbeep/AutoDevice$State";
  jclass jAutoDeviceState = env->FindClass(className.c_str());
  jobject returnValue = nullptr;
  jfieldID field = nullptr;
  auto signatureFun = [&]() -> string { return "L" + className + ";"; };

  switch (state) {
  case AutoDeviceState::invalid:
    field = env->GetStaticFieldID(jAutoDeviceState, "invalid", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::sessionOpened:
    field = env->GetStaticFieldID(jAutoDeviceState, "sessionOpened", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::sessionClosed:
    field = env->GetStaticFieldID(jAutoDeviceState, "sessionClosed", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::waitingForBarcodes:
    field = env->GetStaticFieldID(jAutoDeviceState, "waitingForBarcodes", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::waitingForPaymentResult:
    field = env->GetStaticFieldID(jAutoDeviceState, "waitingForPaymentResult", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::waitingForConfirmation:
    field = env->GetStaticFieldID(jAutoDeviceState, "waitingForConfirmation", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  case AutoDeviceState::waitingForPaymentToken:
    field = env->GetStaticFieldID(jAutoDeviceState, "waitingForPaymentToken", signatureFun().c_str());
    returnValue = env->GetStaticObjectField(jAutoDeviceState, field);
    break;
  }

  return returnValue;
}

void JniUtils::storeJObject(JNIEnv* env, jobject object, AutoDevice* autoDevice) {
  autoDevice->opaque = env->NewGlobalRef(object);
}
void JniUtils::releaseJObject(JNIEnv* env, AutoDevice* autoDevice) {
  env->DeleteGlobalRef(JniUtils::getJObject(autoDevice));
  autoDevice->opaque = nullptr;
}

jobject JniUtils::getJObject(AutoDevice* autoDevice) {
  return (jobject)(autoDevice->opaque);
}