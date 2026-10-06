// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsi_xl_plus/xlplus_runtime.h"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace dsi_xl_plus {
namespace {
Code errnoCode() { return errno == ENOENT ? Code::Missing : errno == EEXIST ? Code::Exists : Code::IoError; }
class NdsStorage final : public Storage {
public:
    Device device = Device::Unavailable;
    Code path(FileId file, char (&out)[80]) {
        const char* root = deviceRoot(device);
        const char* name = fileName(file);
        if (!root || !name) return Code::IoError;
        struct ::stat info;
        if (::stat(root, &info) != 0) return errnoCode();
        if (!S_ISDIR(info.st_mode)) return Code::NotRegular;
        const int length = std::snprintf(out, sizeof(out), "%s%s", root, name);
        return length >= 0 && static_cast<std::size_t>(length) < sizeof(out) ? Code::Ok : Code::IoError;
    }
    Code stat(FileId file, FileInfo& info) override {
        char name[80];
        Code code = path(file, name);
        if (code != Code::Ok) return code;
        struct ::stat st;
        if (::stat(name, &st) != 0) return errnoCode();
        if (st.st_size < 0) return Code::IoError;
        info = {static_cast<std::size_t>(st.st_size), S_ISREG(st.st_mode)};
        return Code::Ok;
    }
    Code open(FileId file, OpenMode mode, Handle& handle) override {
        handle = nullptr;
        char name[80];
        Code code = path(file, name);
        if (code != Code::Ok) return code;
        if (mode == OpenMode::CreateExclusive) {
            if (file != FileId::Stage) return Code::IoError;
            const int fd = ::open(name, O_WRONLY | O_CREAT | O_EXCL, 0600);
            if (fd < 0) return errnoCode();
            FILE* stream = ::fdopen(fd, "wb");
            if (!stream) { ::close(fd); return Code::IoError; }
            handle = stream;
        } else {
            FileInfo info;
            code = stat(file, info);
            if (code != Code::Ok) return code;
            if (!info.regular) return Code::NotRegular;
            handle = std::fopen(name, "rb");
            if (!handle) return errnoCode();
        }
        return Code::Ok;
    }
    Code read(Handle handle, void* data, std::size_t capacity, std::size_t& count) override {
        auto* stream = static_cast<FILE*>(handle);
        count = std::fread(data, 1, capacity, stream);
        return std::ferror(stream) ? Code::IoError : Code::Ok;
    }
    Code write(Handle handle, const void* data, std::size_t size, std::size_t& count) override {
        auto* stream = static_cast<FILE*>(handle);
        count = std::fwrite(data, 1, size, stream);
        return std::ferror(stream) ? Code::IoError : Code::Ok;
    }
    Code flush(Handle handle) override {
        auto* stream = static_cast<FILE*>(handle);
        if (std::fflush(stream) != 0) return Code::IoError;
        return ::fsync(::fileno(stream)) == 0 ? Code::Ok : Code::IoError;
    }
    Code close(Handle handle) override {
        return std::fclose(static_cast<FILE*>(handle)) == 0 ? Code::Ok : Code::IoError;
    }
    Code renameNoReplace(FileId from, FileId to) override {
        const auto n = static_cast<unsigned>(to);
        const bool activeBad = from == FileId::Active && n >= static_cast<unsigned>(FileId::BadActive0) && n <= static_cast<unsigned>(FileId::BadActive3);
        const bool knownBad = from == FileId::KnownGood && n >= static_cast<unsigned>(FileId::BadKnownGood0) && n <= static_cast<unsigned>(FileId::BadKnownGood3);
        if (!(activeBad || knownBad || (from == FileId::KnownGood && to == FileId::Previous) ||
              (from == FileId::Stage && to == FileId::KnownGood))) return Code::IoError;
        FileInfo info;
        Code code = stat(from, info);
        if (code != Code::Ok) return code;
        if (!info.regular) return Code::NotRegular;
        code = stat(to, info);
        if (code != Code::Missing) return code == Code::Ok ? Code::Exists : code;
        char source[80], destination[80];
        code = path(from, source);
        if (code == Code::Ok) code = path(to, destination);
        if (code != Code::Ok) return code;
        // Pinned libfat also rejects existing rename destinations (EEXIST).
        return ::rename(source, destination) == 0 ? Code::Ok : errnoCode();
    }
    Code removePrevious() override {
        FileInfo info;
        Code code = stat(FileId::Previous, info);
        if (code != Code::Ok) return code;
        if (!info.regular) return Code::NotRegular;
        char name[80];
        code = path(FileId::Previous, name);
        if (code != Code::Ok) return code;
        return ::unlink(name) == 0 ? Code::Ok : errnoCode();
    }
};
}
Storage& ndsStorage(Device device) {
    static NdsStorage storage; // No I/O in construction.
    storage.device = device;
    return storage;
}
}
