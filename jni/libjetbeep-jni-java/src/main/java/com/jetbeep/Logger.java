package com.jetbeep;

import java.util.Map;
import java.util.HashMap;

public class Logger {
  static {
    System.loadLibrary("jetbeep-jni");
    Logger.initJvm();
  }

  public enum Level {
    verbose(0),
    debug(1),
    info(2),
    warning(3),
    error(4),
    silent(5);

    private int value;
    Level(int value) {
      this.value = value;
    }

    static Map<Integer, Level> map = new HashMap<>();
    static {
      for (Level logLevel : Level.values()) {
        map.put(logLevel.value, logLevel);
     }
    }
    
    public int getValue() {
      return value;
    }

    public static Level getByValue(int value) {
      if (!map.containsKey(value)) {
        return Level.silent;
      } else {
        return map.get(value);
      }      
   }
  }

  static public Level getLogLevel() {
    Level level = Level.getByValue(nativeGetLogLevel());
    return level;
  }  

  static public void setLogLevel(Level level) {
    nativeSetLogLevel(level.getValue());
  }

  static public native void setCoutEnabled(boolean enabled);
  static public native void setCerrEnabled(boolean enabled);

  private static native int nativeGetLogLevel();
  private static native void nativeSetLogLevel(int level);
  private static native void initJvm();
}