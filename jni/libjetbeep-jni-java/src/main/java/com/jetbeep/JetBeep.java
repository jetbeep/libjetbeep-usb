package com.jetbeep;

public class JetBeep {
  public native int test(int a);
  static {
    System.loadLibrary("jetbeep-jni");
  }

}