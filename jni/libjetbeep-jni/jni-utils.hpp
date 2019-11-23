#include "../../lib/libjetbeep.hpp"
#include <jni.h>
#include <string>

namespace JetBeep {
  class JniUtils {
  public:
    static void throwIllegalStateException(JNIEnv* env, const std::string& message);
    static void throwNullPointerException(JNIEnv* env, const std::string& message);
    static bool getAutoDevicePointer(JNIEnv* env, jlong ptr, AutoDevice** autoDevice);
    static std::string getString(JNIEnv* env, jstring string);
    static void storeJvm(JNIEnv* env);
    static JavaVM* getJvm();
    static JNIEnv* attachCurrentThread();
    static void detachCurrentThread();
    static jobject convertAutoDeviceState(JNIEnv* env, const AutoDeviceState& state);

  private:
    static Logger m_log;
    static JavaVM* m_jvm;
  };
} // namespace JetBeep