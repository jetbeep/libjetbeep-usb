/*
 * logger.h
 *
 *  Created on: Oct 1, 2019
 *      Author: oleh
 */

#ifndef LIB_UTILS_LOGGER_HPP_
#define LIB_UTILS_LOGGER_HPP_

#include <iostream>
#include <string>
#include <sstream>
#include <functional>

namespace JetBeep {
  enum class LoggerLevel : int { verbose = 0, debug, info, warning, error, silent };

  typedef std::function<void(const std::string&)> LoggerLineCallback;

  class Logger {
  public:
    Logger(const char* module_name);

    Logger& v();
    Logger& d();
    Logger& i();
    Logger& w();
    Logger& e();

    static LoggerLevel level;
    static Logger& endl(Logger& a);

    static bool coutEnabled;
    static bool cerrEnabled;
    static bool externalOutputEnabled;

    static LoggerLineCallback outputCallback;

    typedef Logger& (*logger_manipulator)(Logger&);
    Logger& operator<<(logger_manipulator manip) {
      // call the function, and return it's value
      return manip(*this);
    }

    template <class T>
    Logger& operator<<(const T& t) {
      if (Logger::m_threadLevel >= Logger::level) {
        coutValue(t);
        cerrValue(t);
        externalOutputValue(t);
      }

      return *this;
    }

  private:
    std::string m_module_name;
    static std::ostringstream m_str;

    Logger& output();

    static thread_local LoggerLevel m_threadLevel;

    template <class T>
    void coutValue(T t) {
      if (!coutEnabled) {
        return;
      }

      std::cout << t;
    }

    template <class T>
    void cerrValue(T t) {
      if (!cerrEnabled) {
        return;
      }

      std::cerr << t;
    }

    template <class T>
    void externalOutputValue(T t) {
      if (!externalOutputEnabled) {
        return;
      }

      Logger::m_str << t;
    }
  };
} // namespace JetBeep

#endif /* LIB_UTILS_LOGGER_HPP_ */
