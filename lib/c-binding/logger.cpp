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
JETBEEP_API void jetbeep_logger_set_cout_enabled(bool enabled) {
  Logger::coutEnabled = enabled;
}
JETBEEP_API void jetbeep_logger_set_cerr_enabled(bool enabled) {
  Logger::cerrEnabled = enabled;
}