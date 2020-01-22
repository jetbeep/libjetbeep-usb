#define JETBEEP_API_EXPORTS
#include "libjetbeep.h"
#include "libjetbeep.hpp"

using namespace JetBeep;

JETBEEP_API autodevice_handle_t jetbeep_autodevice_new() {
    return (autodevice_handle_t) new AutoDevice;
}