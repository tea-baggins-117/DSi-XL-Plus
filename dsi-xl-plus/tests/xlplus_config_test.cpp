// SPDX-License-Identifier: GPL-3.0-or-later
#include "xlplus_test_storage.h"
#include <iostream>

using namespace dsi_xl_plus;
using namespace dsi_xl_plus::test;
int main() {
    assert(parse(disabled).code == Code::Ok && !parse(disabled).config.enabled);
    assert(parse(enabled).config.enabled && parse(enabled).config.diagnostics);
    assert(parse("[DSiXLPlus]\nschema=1").code == Code::Ok);
    assert(parse("\xEF\xBB\xBF[DSiXLPlus]\r\nschema = 1\r\nenabled=1\r\n").config.enabled);
    assert(parse("[future]\nvalue=x\n[DSiXLPlus]\nschema=1\nextra=y\n").code == Code::Ok);
    const std::vector<std::string> malformed = {
        "", "# empty\n", "[DSiXLPlus]\nenabled=1\n", "[DSiXLPlus\nschema=1\n",
        "schema=1\n", "[DSiXLPlus]\nschema=1x\n", "[DSiXLPlus]\nschema=+1\n",
        "[DSiXLPlus]\nschema=1\nenabled=true\n", "[DSiXLPlus]\nschema=1\ndiagnostics=2\n",
        disabled + "schema=1\n", disabled + "enabled=1\n", disabled + "diagnostics=1\n",
        disabled + std::string(33, 'k') + "=x\n", disabled + "extra=" + std::string(65, 'x') + "\n",
        disabled + "[bad section]\nx=y\n", disabled + std::string("x=\0y\n", 5),
        disabled + "x=\x7f\n", disabled + "x=\x01\n", "\xEF\xBB", disabled + "[DSiXLPlus]\nenabled=1\n"
    };
    for (const auto& data : malformed) {
        const auto result = parse(data);
        assert(result.code == Code::Malformed);
        assert(!result.config.enabled && !result.config.diagnostics);
    }
    assert(parse("[DSiXLPlus]\nschema=2\nenabled=new-format\n").code == Code::UnsupportedSchema);
    assert(parse("[DSiXLPlus]\nschema=999999999999999999999999999\n").code == Code::UnsupportedSchema);
    assert(parse(disabled + std::string(32, 'k') + "=" + std::string(64, 'v') + "\n").code == Code::Ok);
    assert(parse(disabled + "#" + std::string(254, 'x') + "\n").code == Code::Ok);
    assert(parse(disabled + "#" + std::string(255, 'x') + "\n").code == Code::Malformed);
    assert(parse("[DSiXLPlus]\nschema=1\n" + std::string(126, '\n')).code == Code::Ok);
    assert(parse("[DSiXLPlus]\nschema=1\n" + std::string(127, '\n')).code == Code::Malformed);
    std::string max = "[DSiXLPlus]\nschema=1\n";
    while (max.size() + 256 <= kMaxConfigBytes) max += "#" + std::string(254, 'x') + "\n";
    if (max.size() < kMaxConfigBytes) max += "#" + std::string(kMaxConfigBytes - max.size() - 1, 'x');
    assert(max.size() == kMaxConfigBytes && parse(max).code == Code::Ok);
    assert(parse(max + "x").code == Code::TooLarge);
    FakeStorage partial;
    partial.files[FileId::Active] = enabled;
    partial.maxRead = 1;
    assert(readConfig(partial, FileId::Active).config.enabled);
    const auto calls = partial.calls;
    for (unsigned i = 1; i <= calls; ++i) {
        FakeStorage fault;
        fault.files[FileId::Active] = enabled;
        fault.maxRead = 1;
        fault.failAt = i;
        auto result = readConfig(fault, FileId::Active);
        assert(result.code == Code::IoError && !result.config.enabled);
        assert(!fault.openHandles);
    }
    // Deterministic malformed/truncated byte corpus; no allocations in the parser.
    std::uint32_t rng = 0xD51;
    for (int i = 0; i < 2000; ++i) {
        std::string data;
        for (int n = i % 400; n; --n) { rng = rng * 1664525 + 1013904223; data += static_cast<char>(rng >> 24); }
        const auto result = parse(data);
        if (result.code != Code::Ok) assert(!result.config.enabled && !result.config.diagnostics);
    }
    std::cout << "PASS config: syntax, bounds, partial reads, every read/close failure, 2000 byte inputs\n";
}
