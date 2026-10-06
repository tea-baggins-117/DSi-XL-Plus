// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsi_xl_plus/xlplus_config.h"
#include "dsi_xl_plus/xlplus_version.h"
#include <cstring>

namespace dsi_xl_plus {
namespace {
bool whitespace(char c) { return c == ' ' || c == '\t'; }
char* trim(char* text) {
    while (whitespace(*text)) ++text;
    std::size_t length = std::strlen(text);
    while (length && whitespace(text[length - 1])) text[--length] = 0;
    return text;
}
bool nameValid(const char* text, std::size_t limit) {
    const std::size_t length = std::strlen(text);
    if (!length || length > limit) return false;
    for (; *text; ++text) {
        const char c = *text;
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) return false;
    }
    return true;
}
struct Parser {
    char line[kMaxLineBytes + 1]{};
    std::size_t length = 0, lines = 0;
    unsigned seen = 0;
    bool inSection = false, haveSection = false, invalid = false;
    bool unsupported = false, previousCR = false;
    Config config{};

    void finishLine() {
        if (++lines > kMaxLines) invalid = true;
        line[length] = 0;
        char* text = trim(line);
        if (*text && *text != '#' && *text != ';') {
            if (*text == '[') {
                const std::size_t n = std::strlen(text);
                if (n < 3 || text[n - 1] != ']') invalid = true;
                else {
                    text[n - 1] = 0;
                    char* section = trim(text + 1);
                    if (!nameValid(section, kMaxKeyBytes)) invalid = true;
                    inSection = std::strcmp(section, "DSiXLPlus") == 0;
                    haveSection = true;
                }
            } else {
                char* separator = std::strchr(text, '=');
                if (!haveSection || !separator) invalid = true;
                else {
                    *separator = 0;
                    char* key = trim(text);
                    char* value = trim(separator + 1);
                    if (!nameValid(key, kMaxKeyBytes) || std::strlen(value) > kMaxValueBytes) invalid = true;
                    if (inSection) {
                        unsigned bit = std::strcmp(key, "schema") == 0 ? 1 :
                                       std::strcmp(key, "enabled") == 0 ? 2 :
                                       std::strcmp(key, "diagnostics") == 0 ? 4 : 0;
                        if (seen & bit) invalid = true;
                        seen |= bit;
                        if (bit == 1) {
                            // No integer conversion overflow, signs or partial numeric parsing.
                            bool numeric = *value != 0;
                            for (char* p = value; *p; ++p) numeric &= *p >= '0' && *p <= '9';
                            if (!numeric) invalid = true;
                            else if (std::strcmp(value, "1") != 0) unsupported = true;
                        } else if (bit) {
                            if (std::strcmp(value, "0") && std::strcmp(value, "1")) invalid = true;
                            else if (bit == 2) config.enabled = *value == '1';
                            else config.diagnostics = *value == '1';
                        }
                    }
                }
            }
        }
        length = 0;
    }
    void feed(unsigned char c) {
        if (c == '\n' && previousCR) { previousCR = false; return; }
        previousCR = c == '\r';
        if (c == '\r' || c == '\n') { finishLine(); return; }
        if (c == 0 || (c < 32 && c != '\t') || c == 127) { invalid = true; return; }
        if (length == kMaxLineBytes) { invalid = true; return; }
        line[length++] = static_cast<char>(c);
    }
};
}

ConfigResult readConfig(Storage& storage, FileId file) {
    ConfigResult result;
    FileInfo info;
    result.code = storage.stat(file, info);
    if (result.code != Code::Ok) return result;
    if (!info.regular) { result.code = Code::NotRegular; return result; }
    if (info.size > kMaxConfigBytes) { result.code = Code::TooLarge; return result; }
    Handle handle = nullptr;
    result.code = storage.open(file, OpenMode::Read, handle);
    if (result.code != Code::Ok) return result;
    Parser parser;
    unsigned char prefix[3]{};
    std::size_t prefixLength = 0;
    unsigned char buffer[256];
    bool eof = false;
    while (result.code == Code::Ok) {
        std::size_t count = 0;
        const std::size_t remaining = kMaxConfigBytes + 1 - result.stamp.bytes;
        result.code = storage.read(handle, buffer, remaining < sizeof(buffer) ? remaining : sizeof(buffer), count);
        if (result.code != Code::Ok) break;
        if (!count) { eof = true; break; }
        if (count > remaining || count > sizeof(buffer)) { result.code = Code::IoError; break; }
        for (std::size_t i = 0; i < count; ++i) {
            result.stamp.hash ^= buffer[i];
            result.stamp.hash *= UINT64_C(1099511628211);
            ++result.stamp.bytes;
            if (prefixLength < 3) {
                prefix[prefixLength++] = buffer[i];
                if (prefixLength == 3 && !(prefix[0] == 0xEF && prefix[1] == 0xBB && prefix[2] == 0xBF))
                    for (auto c : prefix) parser.feed(c);
            } else parser.feed(buffer[i]);
        }
        if (result.stamp.bytes > kMaxConfigBytes) result.code = Code::TooLarge;
    }
    if (storage.close(handle) != Code::Ok) result.code = Code::IoError;
    if (result.code != Code::Ok) return result;
    if (!eof || result.stamp.bytes != info.size) { result.code = Code::Changed; return result; }
    result.stamp.complete = true;
    if (prefixLength < 3) for (std::size_t i = 0; i < prefixLength; ++i) parser.feed(prefix[i]);
    if (parser.length) parser.finishLine();
    // Never quarantine a recognizable newer schema, even if its fields differ.
    result.code = parser.unsupported ? Code::UnsupportedSchema :
                  parser.invalid || !(parser.seen & 1) ? Code::Malformed : Code::Ok;
    if (result.code == Code::Ok) result.config = parser.config;
    return result;
}
}
