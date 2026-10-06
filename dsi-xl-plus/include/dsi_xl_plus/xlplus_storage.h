// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
#include <cstdint>

namespace dsi_xl_plus {
enum class Code : unsigned char {
    Ok, Missing, IoError, NotRegular, TooLarge, Malformed, UnsupportedSchema,
    Exists, Changed, Skipped, Unchanged, Promoted, Quarantined, QuarantineFull
};
enum class Device : unsigned char { Unavailable, Sd, Flashcard };
enum class FileId : unsigned char {
    Active, KnownGood, Previous, Stage,
    BadActive0, BadActive1, BadActive2, BadActive3,
    BadKnownGood0, BadKnownGood1, BadKnownGood2, BadKnownGood3, Count
};
enum class OpenMode : unsigned char { Read, CreateExclusive };
using Handle = void*;
struct FileInfo { std::size_t size = 0; bool regular = false; };

// Only fixed owned config identifiers cross this boundary, never arbitrary paths.
class Storage {
public:
    virtual ~Storage() = default;
    virtual Code stat(FileId file, FileInfo& info) = 0;
    virtual Code open(FileId file, OpenMode mode, Handle& handle) = 0;
    virtual Code read(Handle handle, void* data, std::size_t capacity, std::size_t& count) = 0;
    virtual Code write(Handle handle, const void* data, std::size_t size, std::size_t& count) = 0;
    virtual Code flush(Handle handle) = 0;
    virtual Code close(Handle handle) = 0;
    virtual Code renameNoReplace(FileId from, FileId to) = 0;
    virtual Code removePrevious() = 0;
};

struct Stamp {
    std::uint64_t hash = UINT64_C(14695981039346656037);
    std::size_t bytes = 0;
    bool complete = false;
};
bool sameStamp(const Stamp& a, const Stamp& b);
const char* codeName(Code code);
const char* fileName(FileId file);
const char* deviceRoot(Device device);
Code compareFiles(Storage& storage, FileId a, FileId b, bool& equal);
Code quarantine(Storage& storage, FileId file, const Stamp& expected);
Code promoteKnownGood(Storage& storage, const Stamp& expected);
}
