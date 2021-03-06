#define JETBEEP_API_EXPORTS
#include "../libjetbeep.h"
#include "../libjetbeep.hpp"

using namespace JetBeep;

JETBEEP_API jetbeep_logger_level_t jetbeep_logger_get_level() {
  return (jetbeep_logger_level_t)Logger::level;
}
JETBEEP_API void jetbeep_logger_set_level(jetbeep_logger_level_t level) {
  Logger::level = (LoggerLevel)level;
}
JETBEEP_API bool jetbeep_logger_is_cout_enabled() {
  return Logger::coutEnabled;
}
JETBEEP_API bool jetbeep_logger_is_cerr_enabled() {
  return Logger::cerrEnabled;
}
JETBEEP_API bool jetbeep_logger_is_external_output_enabled() {
  return Logger::externalOutputEnabled;
}
JETBEEP_API void jetbeep_logger_set_cout_enabled(bool enabled) {
  Logger::coutEnabled = enabled;
}
JETBEEP_API void jetbeep_logger_set_cerr_enabled(bool enabled) {
  Logger::cerrEnabled = enabled;
}
JETBEEP_API void jetbeep_logger_set_external_output_enabled(bool enabled) {
  Logger::externalOutputEnabled = enabled;
}

JETBEEP_API void jetbeep_logger_set_external_output_callback(jetbeep_logger_line_callback_t callback, void *data) {
  Logger::outputCallback = [callback, data](const std::string& line) { callback(line.c_str(), data); };
}