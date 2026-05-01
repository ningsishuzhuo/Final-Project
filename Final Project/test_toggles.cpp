#include "test_toggles.h"

namespace TestToggles {
namespace {

RuntimeFlags g_runtimeFlags = {};

}  

const RuntimeFlags& Get() {
    return g_runtimeFlags;
}

RuntimeFlags& Mutable() {
    return g_runtimeFlags;
}

void ResetDefaults() {
    g_runtimeFlags = RuntimeFlags{};
}

}  
