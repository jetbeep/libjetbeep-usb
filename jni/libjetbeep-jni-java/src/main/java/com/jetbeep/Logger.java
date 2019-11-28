package com.jetbeep;

import java.util.Map;
import java.util.HashMap;

public class Logger {
  static {
    System.loadLibrary("jetbeep-jni");
    Logger.initJvm();
  }
/**
 * Log-level of libjetbeep-jni
 */
  public enum Level {
    /**
     * all messages
     */
    verbose(0),
    /**
     * only debug, info, warning and error messages
     */
    debug(1),
    /**
     * only info, warning and error messages
     */
    info(2),
    /**
     * warning and error messages
     */
    warning(3),
    /**
     * only error messages
     */    
    error(4),
    /**
     * no logging
     */
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

  
  /** 
   * <p>Returns current log-level of libjetbeep-jni.</p>
   * <p>Default: Level.silent</p>
   * @return Level - log-level
   */
  static public Level getLogLevel() {
    Level level = Level.getByValue(nativeGetLogLevel());
    return level;
  }  

  
  /** 
   * <p>Sets log-level of libjetbeep-jni to a new value. </p>
   * <p>Default: Level.silent </p>
   * @param level - log-level
   */
  static public void setLogLevel(Level level) {
    nativeSetLogLevel(level.getValue());
  }

  /**
   * <p>Enables or disables log output to stdout (System.out) </p>
   * <p>Default: false</p>
   * @param enabled - true(enabled) or false(disabled)
   */
  static public native void setCoutEnabled(boolean enabled);
  /**
   * <p>Enables or disables log output to stderr (System.err) </p>
   * <p>Default: false </p>
   * @param enabled - true(enabled) or false(disabled)
   */  
  static public native void setCerrEnabled(boolean enabled);

  private static native int nativeGetLogLevel();
  private static native void nativeSetLogLevel(int level);
  private static native void initJvm();
}