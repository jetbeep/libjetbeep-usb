#define JETBEEP_API_EXPORTS
#include "libjetbeep.h"
#include "libjetbeep.hpp"

using namespace JetBeep;

JETBEEP_API jetbeep_autodevice_handle_t jetbeep_autodevice_new() {
    return (jetbeep_autodevice_handle_t) new AutoDevice;
}

JETBEEP_API void jetbeep_autodevice_free(jetbeep_autodevice_handle_t handle) {
    auto autodevice = (AutoDevice*)handle;
    delete autodevice;
}