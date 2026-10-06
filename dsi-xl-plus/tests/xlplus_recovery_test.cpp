// SPDX-License-Identifier: GPL-3.0-or-later
#include "xlplus_test_storage.h"
#include <iostream>

using namespace dsi_xl_plus;
using namespace dsi_xl_plus::test;
int main() {
    FakeStorage storage;
    storage.files[FileId::KnownGood] = enabled;
    auto state = loadConfiguration(storage);
    assert(state.source == ConfigSource::Defaults && !optionalFeaturesEnabled(state, true));
    assert(settingsReady(storage, state, true) == Code::Skipped && storage.mutations == 0);
    storage.files[FileId::Active] = disabled;
    state = loadConfiguration(storage);
    assert(state.source == ConfigSource::Active && !optionalFeaturesEnabled(state, true));
    storage.files[FileId::Active] = enabled;
    state = loadConfiguration(storage);
    assert(optionalFeaturesEnabled(state, true) == kFoundationBuilt);
    assert(!optionalFeaturesEnabled(state, false));
    assert(!kDashboardAvailable && !kLibraryAvailable && !kProfilesAvailable && !kSaveManagerAvailable && !kSafeModeAvailable);
    storage.files[FileId::Active] = "bad";
    state = loadConfiguration(storage);
    assert(state.source == ConfigSource::KnownGood && state.config.enabled && state.bypassOptional);
    assert(!optionalFeaturesEnabled(state, true));
    const auto mutations = storage.mutations;
    assert(settingsReady(storage, state, false) == Code::Skipped && mutations == storage.mutations);
    state = loadConfiguration(storage);
    auto code = settingsReady(storage, state, true);
    assert(code == (kFoundationBuilt ? Code::Quarantined : Code::Skipped));
    const auto count = storage.calls;
    assert(settingsReady(storage, state, true) == code && count == storage.calls);
    storage.files[FileId::Active] = "[DSiXLPlus]\nschema=99\n";
    state = loadConfiguration(storage);
    auto before = storage.files;
    assert(settingsReady(storage, state, true) == Code::Skipped && before == storage.files);
    storage.files[FileId::Active] = "bad";
    storage.files[FileId::KnownGood] = "bad lkg";
    storage.files[FileId::Previous] = disabled;
    state = loadConfiguration(storage);
    assert(state.source == ConfigSource::Previous && state.bypassOptional);
    storage.files[FileId::Previous] = "bad prev";
    state = loadConfiguration(storage);
    assert(state.source == ConfigSource::Defaults && !state.config.enabled);
    storage.files[FileId::KnownGood] = enabled;
    storage.nonRegular[FileId::Active] = true;
    state = loadConfiguration(storage);
    assert(state.hadIoError && state.source == ConfigSource::KnownGood);
    before = storage.files;
    assert(settingsReady(storage, state, true) == Code::Skipped && before == storage.files);
    assert(deviceRoot(Device::Unavailable) == nullptr);
    assert(std::string(deviceRoot(Device::Sd)) == "sd:/_nds/DSiXLPlus/");
    assert(std::string(deviceRoot(Device::Flashcard)) == "fat:/_nds/DSiXLPlus/");
    assert(fileName(FileId::Count) == nullptr);
    assert(!storage.openHandles);
    std::cout << "PASS recovery (compile gate " << kFoundationBuilt << "): defaults, fallback order, recovery masking, unsupported schema, checkpoint idempotence\n";
}
