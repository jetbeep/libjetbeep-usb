#include "../../lib/libjetbeep.hpp"
#include <jni.h>
#include <mutex>
#include <string>

#define JSTRING_SIGNATURE "Ljava/lang/String;"

namespace JetBeep {
  class JniUtils {
  public:
    static void throwIllegalStateException(JNIEnv* env, const std::string& message);
    static void throwNullPointerException(JNIEnv* env, const std::string& message);
    static void throwRuntimeException(JNIEnv* env, const std::string& message);
    static void throwIOException(JNIEnv* env, const std::string& message);
    static std::string getString(JNIEnv* env, jstring string);
    static void storeJvm(JNIEnv* env);
    static JavaVM* getJvm();
    static JNIEnv* attachCurrentThread();
    static void detachCurrentThread();
    static jobject convertAutoDeviceState(JNIEnv* env, const AutoDeviceState& state);

    static bool getAutoDevicePointer(JNIEnv* env, jlong ptr, AutoDevice** autoDevice);
    static jobject storeAutoDeviceJObject(JNIEnv* env, jobject object, AutoDevice* autoDevice);
    static void releaseAutoDeviceJObject(JNIEnv* env, AutoDevice* autoDevice);
    static jobject getAutoDeviceJObject(AutoDevice* autoDevice);

    static bool getEasyPayBackendPointer(JNIEnv* env, jlong ptr, EasyPayBackend** backend);
    static void storeEasyPayBackendJObject(JNIEnv* env, jobject object, EasyPayBackend* backend);
    static void releaseEasyPayBackendJObject(JNIEnv* env, EasyPayBackend* backend);
    static jobject getEasyPayBackendJObject(EasyPayBackend* backend);

    static jobject getJCardInfoObj(JNIEnv* env, const NFC::DetectionEventData * detectionEventData);
    static void getMifareClassicKeyFromMFCKey(JNIEnv* env, jobject jMFCKeyObj, NFC::MifareClassic::MifareClassicKey* key_p);
    static jobject getMFCBlockDataFromMifareBlockContent(JNIEnv* env, const NFC::MifareClassic::MifareBlockContent * content_p);
    static void getMifareBlockContentFromMFCBlockData(JNIEnv* env, jobject jBlockDataObj, NFC::MifareClassic::MifareBlockContent * content_p);
    static jobject createMFCOperationException(JNIEnv* env, const exception_ptr& ex);

    static jfieldID getPtrField(JNIEnv *env, jobject object, std::string classAlias = "AutoDevice");
    static std::recursive_mutex mutex;

  private:
    static Logger m_log;
    static JavaVM* m_jvm;
  };
} // namespace JetBeep