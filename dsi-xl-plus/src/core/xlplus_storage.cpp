// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsi_xl_plus/xlplus_storage.h"
#include "dsi_xl_plus/xlplus_config.h"
#include <cstring>

namespace dsi_xl_plus {
bool sameStamp(const Stamp& a, const Stamp& b) {
    // Change detection only, not an authenticity/security check.
    return a.complete && b.complete && a.bytes == b.bytes && a.hash == b.hash;
}
const char* codeName(Code code) {
    switch (code) {
        case Code::Ok: return "Valid";
        case Code::Missing: return "Not present";
        case Code::IoError: return "Storage error";
        case Code::NotRegular: return "Not a regular file";
        case Code::TooLarge: return "Configuration too large";
        case Code::Malformed: return "Invalid configuration";
        case Code::UnsupportedSchema: return "Unsupported schema";
        case Code::Exists: return "Existing file preserved";
        case Code::Changed: return "File changed; deferred";
        case Code::Skipped: return "No maintenance performed";
        case Code::Unchanged: return "Known-good copy unchanged";
        case Code::Promoted: return "Known-good copy verified";
        case Code::Quarantined: return "Invalid file preserved";
        case Code::QuarantineFull: return "Quarantine slots full";
    }
    return "Unavailable";
}
const char* fileName(FileId file) {
    static constexpr const char* names[] = {
        "settings.ini", "settings.lkg.ini", "settings.lkg.prev.ini", "settings.lkg.tmp",
        "settings.bad.0.ini", "settings.bad.1.ini", "settings.bad.2.ini", "settings.bad.3.ini",
        "settings.lkg.bad.0.ini", "settings.lkg.bad.1.ini", "settings.lkg.bad.2.ini", "settings.lkg.bad.3.ini"
    };
    const auto index = static_cast<unsigned>(file);
    return index < static_cast<unsigned>(FileId::Count) ? names[index] : nullptr;
}
const char* deviceRoot(Device device) {
    switch (device) {
        case Device::Sd: return "sd:/_nds/DSiXLPlus/";
        case Device::Flashcard: return "fat:/_nds/DSiXLPlus/";
        default: return nullptr;
    }
}
namespace {
Code closeWith(Storage& storage, Handle handle, Code result) {
    return storage.close(handle) == Code::Ok ? result : Code::IoError;
}
Code readBlock(Storage& storage, Handle handle, unsigned char* buffer,
               std::size_t capacity, std::size_t& count) {
    count = 0;
    while (count < capacity) {
        std::size_t read = 0;
        Code code = storage.read(handle, buffer + count, capacity - count, read);
        if (code != Code::Ok) return code;
        if (read > capacity - count) return Code::IoError;
        if (!read) break;
        count += read;
    }
    return Code::Ok;
}
Code copyToStage(Storage& storage) {
    Handle source = nullptr, stage = nullptr;
    Code code = storage.open(FileId::Active, OpenMode::Read, source);
    if (code != Code::Ok) return code;
    code = storage.open(FileId::Stage, OpenMode::CreateExclusive, stage);
    if (code != Code::Ok) return closeWith(storage, source, code);
    unsigned char buffer[256];
    std::size_t total = 0;
    while (code == Code::Ok) {
        std::size_t count = 0;
        code = storage.read(source, buffer, sizeof(buffer), count);
        if (code != Code::Ok || !count) break;
        if (count > sizeof(buffer) || total + count > kMaxConfigBytes) { code = Code::Changed; break; }
        total += count;
        std::size_t offset = 0;
        while (offset < count && code == Code::Ok) {
            std::size_t written = 0;
            code = storage.write(stage, buffer + offset, count - offset, written);
            if (code == Code::Ok && (!written || written > count - offset)) code = Code::IoError;
            offset += written;
        }
    }
    if (code == Code::Ok) code = storage.flush(stage);
    code = closeWith(storage, stage, code);
    return closeWith(storage, source, code);
}
}
Code compareFiles(Storage& storage, FileId a, FileId b, bool& equal) {
    equal = false;
    Handle first = nullptr, second = nullptr;
    Code code = storage.open(a, OpenMode::Read, first);
    if (code != Code::Ok) return code;
    code = storage.open(b, OpenMode::Read, second);
    if (code != Code::Ok) return closeWith(storage, first, code);
    unsigned char aBytes[256], bBytes[256];
    std::size_t total = 0;
    while (code == Code::Ok) {
        std::size_t aCount = 0, bCount = 0;
        code = readBlock(storage, first, aBytes, sizeof(aBytes), aCount);
        if (code == Code::Ok) code = readBlock(storage, second, bBytes, sizeof(bBytes), bCount);
        if (code != Code::Ok) break;
        total += aCount > bCount ? aCount : bCount;
        if (total > kMaxConfigBytes) { code = Code::TooLarge; break; }
        if (aCount != bCount || std::memcmp(aBytes, bBytes, aCount)) break;
        if (!aCount) { equal = true; break; }
    }
    code = closeWith(storage, second, code);
    code = closeWith(storage, first, code);
    if (code != Code::Ok) equal = false;
    return code;
}
Code quarantine(Storage& storage, FileId file, const Stamp& expected) {
    if (file != FileId::Active && file != FileId::KnownGood) return Code::Skipped;
    const auto fresh = readConfig(storage, file);
    if (fresh.code != Code::Malformed || !sameStamp(fresh.stamp, expected)) return Code::Changed;
    const unsigned first = static_cast<unsigned>(file == FileId::Active ? FileId::BadActive0 : FileId::BadKnownGood0);
    for (unsigned i = 0; i < 4; ++i) {
        const auto destination = static_cast<FileId>(first + i);
        FileInfo info;
        Code code = storage.stat(destination, info);
        if (code == Code::Ok) continue;
        if (code != Code::Missing) return code;
        code = storage.renameNoReplace(file, destination);
        if (code == Code::Exists) continue;
        return code == Code::Ok ? Code::Quarantined : code;
    }
    return Code::QuarantineFull;
}
Code promoteKnownGood(Storage& storage, const Stamp& expected) {
    auto active = readConfig(storage, FileId::Active);
    if (active.code != Code::Ok) return active.code;
    if (!sameStamp(active.stamp, expected)) return Code::Changed;
    auto known = readConfig(storage, FileId::KnownGood);
    if (known.code != Code::Ok && known.code != Code::Missing && known.code != Code::Malformed)
        return known.code; // Preserve newer/oversized/unreadable files without mutation.
    bool equal = false;
    Code code;
    if (known.code == Code::Ok) {
        code = compareFiles(storage, FileId::Active, FileId::KnownGood, equal);
        if (code != Code::Ok) return code;
        if (equal) return Code::Unchanged;
    }
    code = copyToStage(storage); // O_EXCL: an old stage is never truncated.
    if (code != Code::Ok) return code;
    const auto stage = readConfig(storage, FileId::Stage);
    if (stage.code != Code::Ok) return stage.code;
    if (!sameStamp(stage.stamp, expected)) return Code::Changed;
    code = compareFiles(storage, FileId::Active, FileId::Stage, equal);
    if (code != Code::Ok || !equal) return code == Code::Ok ? Code::Changed : code;
    active = readConfig(storage, FileId::Active);
    if (active.code != Code::Ok || !sameStamp(active.stamp, expected)) return Code::Changed;
    const auto rechecked = readConfig(storage, FileId::KnownGood);
    if (rechecked.code != known.code || (known.code != Code::Missing && !sameStamp(rechecked.stamp, known.stamp)))
        return Code::Changed;

    if (known.code == Code::Ok) {
        // The current verified LKG and verified stage both exist before removing
        // the older predecessor. An interruption then leaves a usable candidate.
        FileInfo previous;
        code = storage.stat(FileId::Previous, previous);
        if (code == Code::Ok) {
            if (!previous.regular) return Code::NotRegular;
            code = storage.removePrevious();
        } else if (code == Code::Missing) code = Code::Ok;
        if (code != Code::Ok) return code;
        code = storage.renameNoReplace(FileId::KnownGood, FileId::Previous);
        if (code != Code::Ok) return code;
    } else if (known.code == Code::Malformed) {
        code = quarantine(storage, FileId::KnownGood, known.stamp);
        if (code != Code::Quarantined) return code;
    }
    code = storage.renameNoReplace(FileId::Stage, FileId::KnownGood);
    if (code != Code::Ok) return code;
    const auto installed = readConfig(storage, FileId::KnownGood);
    if (installed.code != Code::Ok || !sameStamp(installed.stamp, expected)) return Code::Changed;
    code = compareFiles(storage, FileId::Active, FileId::KnownGood, equal);
    return code != Code::Ok ? code : equal ? Code::Promoted : Code::Changed;
}
}
