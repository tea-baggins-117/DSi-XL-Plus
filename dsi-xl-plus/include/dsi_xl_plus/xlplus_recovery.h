// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "xlplus_config.h"
#include "xlplus_feature_flags.h"

namespace dsi_xl_plus {
enum class ConfigSource : unsigned char { Defaults, Active, KnownGood, Previous };
struct RecoveryState {
    Config config{};
    ConfigSource source = ConfigSource::Defaults;
    ConfigResult active{};
    bool bypassOptional = true;
    bool hadIoError = false;
    bool maintenanceDone = false;
    Code maintenance = Code::Skipped;
};
RecoveryState loadConfiguration(Storage& storage);
bool optionalFeaturesEnabled(const RecoveryState& state, bool supportedMode);
Code settingsReady(Storage& storage, RecoveryState& state, bool writableMode);
const char* sourceName(ConfigSource source);
}
