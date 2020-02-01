#include <stdio.h>
#include <stdarg.h>
#include <cstring>
#include "logging.h"
#include "../lib/libjetbeep.hpp"

#define MAX_BUF_LEN 512

JetBeep::Logger* p_logger = nullptr;

static std::string format_output(const char* format, va_list args_list) {
  char buffer[MAX_BUF_LEN];
  memset(buffer, '\0', MAX_BUF_LEN);
  vsnprintf(buffer, MAX_BUF_LEN, format, args_list);
  return string(buffer);
}

void logger_error(const char* format, ...) {
  if (!p_logger) {
    throw runtime_error("p_logger is null");
  }
  va_list argptr;
  va_start(argptr, format);
  auto str = format_output(format, argptr);
  p_logger->e() << str << JetBeep::Logger::endl;
  va_end(argptr);
}

void logger_info_1(const char* format, ...) {
  if (!p_logger) {
    throw runtime_error("p_logger is null");
  }
  va_list argptr;
  va_start(argptr, format);
  auto str = format_output(format, argptr);
  p_logger->i() << str << JetBeep::Logger::endl;
  va_end(argptr);
}

void logger_info_2(const char* format, ...) {
  if (!p_logger) {
    throw runtime_error("p_logger is null");
  }
  va_list argptr;
  va_start(argptr, format);
  auto str = format_output(format, argptr);
  p_logger->d() << str << JetBeep::Logger::endl;
  va_end(argptr);
}

void logger_info_3(const char* format, ...) {
  if (!p_logger) {
    throw runtime_error("p_logger is null");
  }
  va_list argptr;
  va_start(argptr, format);
  auto str = format_output(format, argptr);
  p_logger->v() << str << JetBeep::Logger::endl;
  va_end(argptr);
}

void logger_set_backend(void* logger) {
  p_logger = (JetBeep::Logger*)logger;
}

int logger_get_info_level() {
	return LOGGER_INFO_LVL_3; //allways return max, due to unnecessary checks in related code
}