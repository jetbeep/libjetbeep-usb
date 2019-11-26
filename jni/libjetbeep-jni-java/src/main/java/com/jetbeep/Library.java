package com.jetbeep;

import java.lang.RuntimeException;

public class Library {
  static {
    Library.loadAndCheckVersion();
  }

  private Library() {

  }

  public static void loadAndCheckVersion() {
    System.loadLibrary("jetbeep-jni");
    if (!Library.version().equals(Library.getNativeVersion())) {
      throw new RuntimeException("java(" + Library.version() +") and native("+Library.getNativeVersion()+") version mismatch");
    }
  }
  public static final String version() {
    return "0.1.0";
  } 
  public static native String getNativeVersion();
}