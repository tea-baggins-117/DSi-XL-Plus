// SPDX-License-Identifier: GPL-3.0-or-later
#include "xlplus_test_storage.h"
#include "dsi_xl_plus/xlplus_runtime.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>

using namespace dsi_xl_plus;
using namespace dsi_xl_plus::test;
FakeStorage fixture() {
    FakeStorage s;
    s.files[FileId::Active] = enabled;
    s.files[FileId::KnownGood] = disabled;
    s.files[FileId::Previous] = disabled + "# older\n";
    return s;
}
void survivingKnownGood(FakeStorage& s) {
    s.restart();
    s.files[FileId::Active] = "broken after restart";
    const auto state = loadConfiguration(s);
    assert(state.source == ConfigSource::KnownGood || state.source == ConfigSource::Previous);
    assert(!optionalFeaturesEnabled(state, true));
    assert(!s.openHandles);
}
void promotionBoundaryCases(const char* name, const FakeStorage& initial, bool hasPrevious, bool badKnownGood) {
    const auto expected = parse(enabled).stamp;
    auto completed = initial;
    assert(promoteKnownGood(completed, expected) == Code::Promoted);
    const auto operations = completed.calls, mutations = completed.mutations;
    const auto preserved = [&](FakeStorage& state) {
        assert(!state.openHandles && state.files.at(FileId::Active) == enabled);
        if (hasPrevious) assert(state.files.at(FileId::Previous) == initial.files.at(FileId::Previous));
        if (badKnownGood) {
            const auto& bytes = initial.files.at(FileId::KnownGood);
            bool found = state.files.count(FileId::KnownGood) && state.files.at(FileId::KnownGood) == bytes;
            for (unsigned n = static_cast<unsigned>(FileId::BadKnownGood0); n <= static_cast<unsigned>(FileId::BadKnownGood3); ++n) {
                const auto file = static_cast<FileId>(n);
                found |= state.files.count(file) && state.files.at(file) == bytes;
            }
            assert(found); // Invalid evidence survives even if promotion stops.
        }
        state.restart();
        state.files[FileId::Active] = "invalid on next boot";
        const auto loaded = loadConfiguration(state);
        assert(!optionalFeaturesEnabled(loaded, true) && !state.openHandles);
        if (hasPrevious) assert(loaded.source == ConfigSource::KnownGood || loaded.source == ConfigSource::Previous);
        else if (loaded.source == ConfigSource::Defaults) assert(!loaded.config.enabled && !loaded.config.diagnostics);
        else assert(loaded.source == ConfigSource::KnownGood && loaded.config.enabled);
    };
    for (unsigned n = 1; n <= operations; ++n) {
        auto failed = initial;
        failed.failAt = n;
        assert(promoteKnownGood(failed, expected) != Code::Promoted);
        preserved(failed);
    }
    for (unsigned n = 1; n <= mutations; ++n) {
        auto interrupted = initial;
        interrupted.cutAfter = n;
        assert(promoteKnownGood(interrupted, expected) != Code::Promoted);
        preserved(interrupted);
    }
    std::cout << "PASS " << name << ": " << operations << " operation failures, " << mutations << " interruption boundaries\n";
}
int main() {
    auto normal = fixture();
    const auto expected = parse(enabled).stamp;
    assert(promoteKnownGood(normal, expected) == Code::Promoted);
    assert(normal.files[FileId::KnownGood] == enabled && normal.files[FileId::Previous] == disabled);
    assert(normal.files[FileId::Active] == enabled);
    const auto operations = normal.calls, mutations = normal.mutations;
    assert(promoteKnownGood(normal, expected) == Code::Unchanged && normal.mutations == mutations);
    for (unsigned i = 1; i <= operations; ++i) {
        auto failed = fixture();
        failed.failAt = i;
        const auto code = promoteKnownGood(failed, expected);
        assert(code != Code::Promoted);
        assert(failed.files[FileId::Active] == enabled && !failed.openHandles);
        survivingKnownGood(failed);
    }
    for (unsigned i = 1; i <= mutations; ++i) {
        auto interrupted = fixture();
        interrupted.cutAfter = i;
        assert(promoteKnownGood(interrupted, expected) != Code::Promoted);
        assert(interrupted.files[FileId::Active] == enabled);
        survivingKnownGood(interrupted);
    }
    auto shortIo = fixture();
    shortIo.maxRead = 3; shortIo.maxWrite = 2;
    assert(promoteKnownGood(shortIo, expected) == Code::Promoted);
    auto full = fixture(); full.maxWrite = 0;
    assert(promoteKnownGood(full, expected) == Code::IoError);
    assert(full.files[FileId::KnownGood] == disabled);
    auto ro = fixture(); ro.readOnly = true;
    const auto before = ro.files;
    assert(promoteKnownGood(ro, expected) == Code::IoError && ro.files == before);
    auto corrupt = fixture(); corrupt.corruptStage = true;
    assert(promoteKnownGood(corrupt, expected) != Code::Promoted && corrupt.files[FileId::KnownGood] == disabled);
    auto stale = fixture(); stale.files[FileId::Stage] = "unexplained";
    assert(promoteKnownGood(stale, expected) == Code::Exists && stale.files[FileId::Stage] == "unexplained");
    auto changed = fixture(); changed.files[FileId::Active] += "# changed\n";
    assert(promoteKnownGood(changed, expected) == Code::Changed && !changed.mutations);
    FakeStorage first;
    first.files[FileId::Active] = enabled;
    promotionBoundaryCases("first known-good copy", first, false, false);
    assert(promoteKnownGood(first, expected) == Code::Promoted);
    auto onlyPrevious = fixture(); onlyPrevious.files.erase(FileId::KnownGood);
    promotionBoundaryCases("predecessor-only recovery", onlyPrevious, true, false);
    assert(promoteKnownGood(onlyPrevious, expected) == Code::Promoted);
    assert(onlyPrevious.files[FileId::Previous] == disabled + "# older\n");
    auto badLkg = fixture(); badLkg.files[FileId::KnownGood] = "invalid";
    promotionBoundaryCases("invalid known-good quarantine", badLkg, true, true);
    assert(promoteKnownGood(badLkg, expected) == Code::Promoted && badLkg.files[FileId::BadKnownGood0] == "invalid");
    for (unsigned i = 0; i < 4; ++i) badLkg.files[static_cast<FileId>(static_cast<unsigned>(FileId::BadKnownGood0) + i)] = "evidence";
    badLkg.files[FileId::KnownGood] = "bad again";
    assert(promoteKnownGood(badLkg, expected) == Code::QuarantineFull);
    assert(badLkg.files[FileId::KnownGood] == "bad again");
    auto newer = fixture(); newer.files[FileId::KnownGood] = "[DSiXLPlus]\nschema=2\n";
    assert(promoteKnownGood(newer, expected) == Code::UnsupportedSchema && !newer.mutations);
    FakeStorage quarantineTest;
    quarantineTest.files[FileId::Active] = "bad";
    auto stamp = parse("bad").stamp;
    for (unsigned i = 0; i < 4; ++i) quarantineTest.files[static_cast<FileId>(static_cast<unsigned>(FileId::BadActive0) + i)] = "preserve";
    assert(quarantine(quarantineTest, FileId::Active, stamp) == Code::QuarantineFull);
    assert(quarantineTest.files[FileId::Active] == "bad");
    quarantineTest.files.erase(FileId::BadActive2);
    assert(quarantine(quarantineTest, FileId::Active, stamp) == Code::Quarantined);
    assert(quarantineTest.files[FileId::BadActive2] == "bad");

    // Exercise the actual stdio/POSIX adapter against an expendable directory.
    // libfat semantics still require the pinned ARM build and hardware checks.
    const auto previousCwd = std::filesystem::current_path();
    char temp[] = "/tmp/xlplus-storage-XXXXXX";
    assert(mkdtemp(temp));
    std::filesystem::current_path(temp);
    auto& disk = ndsStorage(Device::Sd);
    assert(loadConfiguration(disk).source == ConfigSource::Defaults);
    assert(!std::filesystem::exists("sd:"));
    std::filesystem::create_directories("sd:/_nds/DSiXLPlus");
    { std::ofstream out("sd:/_nds/DSiXLPlus/settings.ini"); out << enabled; }
    assert(promoteKnownGood(disk, expected) == Code::Promoted);
    assert(readConfig(disk, FileId::KnownGood).config.enabled);
    assert(promoteKnownGood(disk, expected) == Code::Unchanged);
    Handle forbidden = nullptr;
    assert(disk.open(FileId::Active, OpenMode::CreateExclusive, forbidden) == Code::IoError);
    assert(disk.renameNoReplace(FileId::KnownGood, FileId::Active) == Code::IoError);
    { std::ofstream out("sd:/_nds/DSiXLPlus/settings.lkg.tmp"); out << "keep"; }
    { std::ofstream out("sd:/_nds/DSiXLPlus/settings.ini"); out << disabled; }
    assert(promoteKnownGood(disk, parse(disabled).stamp) == Code::Exists);
    std::filesystem::current_path(previousCwd);
    std::filesystem::remove_all(temp);
    std::cout << "PASS storage: " << operations << " operation failures, " << mutations << " interruption boundaries, short I/O, quarantine, real adapter\n";
}
