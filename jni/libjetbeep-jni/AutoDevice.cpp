#include "../../lib/libjetbeep.hpp"
#include "com_jetbeep_AutoDevice.h"
#include "jni-utils.hpp"
#include <unordered_map>

using namespace std;
using namespace JetBeep;

JNIEXPORT jlong JNICALL Java_com_jetbeep_AutoDevice_init(JNIEnv* env, jobject object) {
  auto ptr = new AutoDevice();
  return (jlong)(ptr);
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_free(JNIEnv* env, jobject object, jlong ptr) {
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  delete device;
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_start(JNIEnv* env, jobject object, jlong ptr) {
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->start();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to start device");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_stop(JNIEnv* env, jobject object, jlong ptr) {
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->stop();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to stop device");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_openSession(JNIEnv* env, jobject object, jlong ptr) {
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->openSession();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to open session");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_closeSession(JNIEnv* env, jobject object, jlong ptr) {
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->closeSession();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to close session");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_requestBarcodes(JNIEnv* env, jobject object, jlong ptr) {
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->requestBarcodes()
      .then([](vector<Barcode> barcodes) {
        // TODO
      })
      .catchError([](exception_ptr ptr) {
        // TODO
      });
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to close session");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_cancelBarcodes(JNIEnv* env, jobject object, jlong ptr) {
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->cancelBarcodes();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to close session");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_createPaymentToken(
  JNIEnv* env, jobject object, jlong ptr, jint jamount, jstring jtransactionId, jstring jcashierId, jobjectArray jmetadataKeys, jobjectArray jmetadataValues) {
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  PaymentMetadata metadata;
  uint32_t amount = (uint32_t)jamount;
  auto transactionId = JniUtils::getString(env, jtransactionId);
  auto cashierId = JniUtils::getString(env, jcashierId);
  auto metadataSize = env->GetArrayLength(jmetadataKeys);

  for (int i = 0; i < metadataSize; ++i) {
    jstring jkey = (jstring)env->GetObjectArrayElement(jmetadataKeys, i);
    jstring jvalue = (jstring)env->GetObjectArrayElement(jmetadataValues, i);
    string key = JniUtils::getString(env, jkey);
    string value = JniUtils::getString(env, jvalue);

    metadata[key] = value;
  }

  try {
    device->createPaymentToken(amount, transactionId, cashierId, metadata)
      .then([](std::string token) {
        // TODO
      })
      .catchError([](exception_ptr ptr) {
        // TODO
      });
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to close session");
  }
}

JNIEXPORT void JNICALL Java_com_jetbeep_AutoDevice_cancelPayment(JNIEnv* env, jobject object, jlong ptr) {
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return;
  }

  try {
    device->cancelPayment();
  } catch (...) {
    // TODO: handle all exceptions separetely
    JniUtils::throwIllegalStateException(env, "unable to close session");
  }
}

JNIEXPORT jobject JNICALL Java_com_jetbeep_AutoDevice_state(JNIEnv* env, jobject object, jlong ptr) {
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return nullptr;
  }

  auto state = device->state();
  string className = "AutoDevice$State";
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

JNIEXPORT jboolean JNICALL Java_com_jetbeep_AutoDevice_isMobileConnected(JNIEnv* env, jobject object, jlong ptr) {
  AutoDevice* device = nullptr;
  if (!JniUtils::getAutoDevicePointer(env, ptr, &device)) {
    return false;
  }

  return device->isMobileConnected();
}