#ifndef JETBEEP_API_H
#define JETBEEP_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  const char* key;
  const char* value;
} jetbeep_payment_metadata_t;

#ifdef _WIN32
#ifdef JETBEEP_API_EXPORTS
#define JETBEEP_API __declspec(dllexport)
#else
#define JETBEEP_API __declspec(dllimport)
#endif
#else
#define JETBEEP_API
#endif

#ifdef __cplusplus
}
#endif

#endif