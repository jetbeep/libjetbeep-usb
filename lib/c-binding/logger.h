#ifndef JETBEEP_C_LOGGER_H
#define JETBEEP_C_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "api.h"

typedef void (*jetbeep_logger_line_callback_t)(const char* line, void *data);

typedef enum {
  JETBEEP_LOGGER_VERBOSE = 0,
  JETBEEP_LOGGER_DEBUG,
  JETBEEP_LOGGER_INFO,
  JETBEEP_LOGGER_WARNING,
  JETBEEP_LOGGER_ERROR,
  JETBEEP_LOGGER_SILENT
} jetbeep_logger_level_t;

JETBEEP_API jetbeep_logger_level_t jetbeep_logger_get_level();
JETBEEP_API void jetbeep_logger_set_level(jetbeep_logger_level_t level);
JETBEEP_API bool jetbeep_logger_is_cout_enabled();
JETBEEP_API bool jetbeep_logger_is_cerr_enabled();
JETBEEP_API bool jetbeep_logger_is_external_output_enabled();
JETBEEP_API void jetbeep_logger_set_cout_enabled(bool enabled);
JETBEEP_API void jetbeep_logger_set_cerr_enabled(bool enabled);
JETBEEP_API void jetbeep_logger_set_external_output_enabled(bool enabled);

JETBEEP_API void jetbeep_logger_set_external_output_callback(jetbeep_logger_line_callback_t callback, void *data);

#ifdef __cplusplus
}
#endif

#endif