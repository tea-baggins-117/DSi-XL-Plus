// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "xlplus_storage.h"

namespace dsi_xl_plus {
inline constexpr std::size_t kMaxConfigBytes = 8192;
inline constexpr std::size_t kMaxLines = 128;
inline constexpr std::size_t kMaxLineBytes = 255;
inline constexpr std::size_t kMaxKeyBytes = 32;
inline constexpr std::size_t kMaxValueBytes = 64;
struct Config { bool enabled = false; bool diagnostics = false; };
struct ConfigResult {
    Config config{};
    Code code = Code::Missing;
    Stamp stamp{};
};
ConfigResult readConfig(Storage& storage, FileId file);
}
