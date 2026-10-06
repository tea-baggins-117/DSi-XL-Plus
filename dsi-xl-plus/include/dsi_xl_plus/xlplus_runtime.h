// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "xlplus_recovery.h"

namespace dsi_xl_plus {
struct RuntimeContext {
    RecoveryState recovery{};
    Device device = Device::Unavailable;
    bool supportedMode = false;
    bool initialized = false;
};
// Explicit per-executable initialization, after upstream filesystem setup.
void initialize();
const RuntimeContext& runtime();
void settingsCheckpoint();
Storage& ndsStorage(Device device);
}
