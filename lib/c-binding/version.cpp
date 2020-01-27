#define JETBEEP_API_EXPORTS
#include "../libjetbeep.h"
#include "../libjetbeep.hpp"

using namespace JetBeep;

JETBEEP_API const char* jetbeep_get_version() {
    return JETBEEP_VERSION;
}