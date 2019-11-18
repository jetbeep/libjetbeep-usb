#ifndef JETBEEP_PLATFORM__H
#define JETBEEP_PLATFORM__H

#ifdef _WIN32
#define PLATFORM_WIN
#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_MAC
#define PLATFORM_OSX
#else
#error "Unsupported Apple platform"
#endif
#elif __linux__
#define PLATFORM_LINUX
#else
#error "Unsupported platform"
#endif

#endif
