// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsi_xl_plus/xlplus_recovery.h"

namespace dsi_xl_plus {
namespace {
bool ioFailure(Code code) {
    return code == Code::IoError || code == Code::Changed || code == Code::NotRegular;
}
}
RecoveryState loadConfiguration(Storage& storage) {
    RecoveryState state;
    state.active = readConfig(storage, FileId::Active);
    state.hadIoError = ioFailure(state.active.code);
    if (state.active.code == Code::Ok) {
        state.source = ConfigSource::Active;
        state.config = state.active.config;
        state.bypassOptional = false;
        return state;
    }
    // Intentional removal must never resurrect an enabled known-good copy.
    if (state.active.code == Code::Missing) return state;
    const FileId candidates[] = {FileId::KnownGood, FileId::Previous};
    for (const auto file : candidates) {
        const auto fallback = readConfig(storage, file);
        state.hadIoError |= ioFailure(fallback.code);
        if (fallback.code == Code::Ok) {
            state.config = fallback.config;
            state.source = file == FileId::KnownGood ? ConfigSource::KnownGood : ConfigSource::Previous;
            break;
        }
    }
    return state;
}
bool optionalFeaturesEnabled(const RecoveryState& state, bool supportedMode) {
    return kFoundationBuilt && supportedMode && !state.bypassOptional && state.config.enabled;
}
Code settingsReady(Storage& storage, RecoveryState& state, bool writableMode) {
    if (state.maintenanceDone) return state.maintenance;
    state.maintenanceDone = true;
    if (!kFoundationBuilt || !writableMode || state.hadIoError) return state.maintenance;
    if (state.active.code == Code::Ok)
        state.maintenance = promoteKnownGood(storage, state.active.stamp);
    else if (state.active.code == Code::Malformed && state.active.stamp.complete)
        state.maintenance = quarantine(storage, FileId::Active, state.active.stamp);
    return state.maintenance;
}
const char* sourceName(ConfigSource source) {
    switch (source) {
        case ConfigSource::Active: return "Active configuration";
        case ConfigSource::KnownGood: return "Last known good";
        case ConfigSource::Previous: return "Previous known good";
        default: return "Compiled defaults";
    }
}
}
