// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "dsi_xl_plus/xlplus_recovery.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace dsi_xl_plus::test {
inline const std::string disabled = "[DSiXLPlus]\nschema=1\nenabled=0\ndiagnostics=0\n";
inline const std::string enabled = "[DSiXLPlus]\nschema=1\nenabled=1\ndiagnostics=1\n";
class FakeStorage final : public Storage {
    struct Cursor { FileId file; std::size_t position = 0; bool write = false; };
public:
    std::map<FileId, std::string> files;
    std::vector<std::string> operations;
    std::map<FileId, bool> nonRegular;
    std::size_t maxRead = 256, maxWrite = 256;
    unsigned failAt = 0, calls = 0, mutations = 0, cutAfter = 0, openHandles = 0;
    bool dead = false, readOnly = false, corruptStage = false;
    bool step(const char* name) {
        operations.emplace_back(name);
        return !dead && ++calls != failAt;
    }
    Code mutated() {
        ++mutations;
        if (cutAfter && mutations == cutAfter) dead = true;
        return dead ? Code::IoError : Code::Ok;
    }
    Code stat(FileId file, FileInfo& info) override {
        if (!step("stat")) return Code::IoError;
        auto found = files.find(file);
        if (found == files.end()) return Code::Missing;
        info = {found->second.size(), !nonRegular[file]};
        return Code::Ok;
    }
    Code open(FileId file, OpenMode mode, Handle& handle) override {
        handle = nullptr;
        if (!step("open")) return Code::IoError;
        if (mode == OpenMode::CreateExclusive) {
            if (readOnly) return Code::IoError;
            if (files.count(file)) return Code::Exists;
            files[file] = "";
            if (mutated() != Code::Ok) return Code::IoError;
        } else if (!files.count(file)) return Code::Missing;
        if (nonRegular[file]) return Code::NotRegular;
        handle = new Cursor{file, 0, mode == OpenMode::CreateExclusive};
        ++openHandles;
        return Code::Ok;
    }
    Code read(Handle handle, void* data, std::size_t capacity, std::size_t& count) override {
        count = 0;
        if (!step("read")) return Code::IoError;
        auto& cursor = *static_cast<Cursor*>(handle);
        const auto& bytes = files.at(cursor.file);
        count = std::min({capacity, maxRead, bytes.size() - cursor.position});
        std::memcpy(data, bytes.data() + cursor.position, count);
        cursor.position += count;
        return Code::Ok;
    }
    Code write(Handle handle, const void* data, std::size_t size, std::size_t& count) override {
        count = 0;
        if (!step("write") || readOnly) return Code::IoError;
        auto& cursor = *static_cast<Cursor*>(handle);
        assert(cursor.write);
        count = std::min(size, maxWrite);
        files[cursor.file].append(static_cast<const char*>(data), count);
        if (corruptStage && count) files[cursor.file].back() = '!';
        return mutated();
    }
    Code flush(Handle) override {
        if (!step("flush") || readOnly) return Code::IoError;
        return mutated();
    }
    Code close(Handle handle) override {
        assert(handle && openHandles);
        const bool ok = step("close");
        const bool writable = static_cast<Cursor*>(handle)->write;
        delete static_cast<Cursor*>(handle);
        --openHandles;
        if (!ok) return Code::IoError;
        return writable ? mutated() : Code::Ok;
    }
    Code renameNoReplace(FileId from, FileId to) override {
        if (!step("rename") || readOnly) return Code::IoError;
        if (files.count(to)) return Code::Exists;
        auto found = files.find(from);
        if (found == files.end()) return Code::Missing;
        files[to] = found->second;
        files.erase(found);
        return mutated();
    }
    Code removePrevious() override {
        if (!step("remove") || readOnly) return Code::IoError;
        if (!files.erase(FileId::Previous)) return Code::Missing;
        return mutated();
    }
    void restart() { failAt = calls = cutAfter = 0; dead = false; }
};
inline ConfigResult parse(const std::string& data) {
    FakeStorage storage;
    storage.files[FileId::Active] = data;
    auto result = readConfig(storage, FileId::Active);
    assert(!storage.openHandles && !storage.mutations);
    return result;
}
}
