#pragma once
 
#ifndef _INC_LOGGING
#define _INC_LOGGING

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#define LOGGER_INFO_LVL_0       0
#define LOGGER_INFO_LVL_1       1
#define LOGGER_INFO_LVL_2       2
#define LOGGER_INFO_LVL_3       3


void logger_error(const char* format, ...);

void logger_info_1(const char* format, ...);
void logger_info_2(const char* format, ...);
void logger_info_3(const char* format, ...);

int logger_get_info_level(void);
void logger_set_backend(void *);

void logger_progress_start(void * p_object, uint32_t total_size);
void logger_progress_log(uint32_t size, uint32_t pos);
void logger_progress_end(int error_code);

#ifdef __cplusplus
}   /* ... extern "C" */
#endif  /* __cplusplus */


#endif // _INC_LOGGING
