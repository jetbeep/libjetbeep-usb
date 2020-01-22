#ifndef LIBJETBEEP_H
#define LIBJETBEEP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifdef JETBEEP_API_EXPORTS
#define JETBEEP_API __declspec(dllexport)
#else
#define JETBEEP_API __declspec(dllimport)
#endif
#else
#define JETBEEP_API
#endif

typedef void* autodevice_handle_t;

JETBEEP_API autodevice_handle_t jetbeep_autodevice_new();

#ifdef __cplusplus
}
#endif

#endif
