#ifndef JETBEEP_API_H
#define JETBEEP_API_H

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

#ifdef __cplusplus
}
#endif

#endif