// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsi_xl_plus/xlplus_settings_page.h"
#include "dsi_xl_plus/xlplus_runtime.h"
#include "dsi_xl_plus/xlplus_version.h"
#include "settingsgui.h"
#include <cstdio>

namespace dsi_xl_plus {
namespace {
std::optional<Option> recoveryDetails() {
    char description[240];
    const auto& state = runtime().recovery;
    std::snprintf(description, sizeof(description), "%s. Active file: %s. Maintenance: %s. Dedicated Safe Mode is not implemented.",
                  sourceName(state.source), codeName(state.active.code), codeName(state.maintenance));
    return Option("Configuration status", description, Option::Nul(), {"Read only"}, {0});
}
}
void addSettingsPage() {
    SettingsPage page(kProduct);
    page.option("DSi XL+ version", "DSi XL+ foundation. Based on TWiLight Menu++. Hardware smoke testing is pending.",
                Option::Nul(), {kVersion}, {0});
    page.option("Upstream baseline", "TWiLight Menu++ base commit: 97b22fe2cc282cee5bae7 c12e0db54c1aa111dfb. Preserve upstream credits and licenses.",
                Option::Nul(), {"97b22fe"}, {0});
    const auto& context = runtime();
    const auto& state = context.recovery;
    page.option("Optional features", "Phase 1 provides the foundation only. Dashboard, library and save features are not implemented.",
                Option::Nul(), {optionalFeaturesEnabled(state, context.supportedMode) ? "Requested" : "Disabled"}, {0});
    page.option("Configuration status", "Press A to view configuration fallback and the current maintenance result.",
                Option::Nul(recoveryDetails), {"View"}, {0});
    if (state.config.diagnostics) {
        page.option("Configuration device", "Selected from the launcher device. Configuration never switches to the browsed ROM device.",
                    Option::Nul(), {context.device == Device::Sd ? "SD" : context.device == Device::Flashcard ? "Flashcard" : "Unavailable"}, {0});
        page.option("Recovery boundary", "Checked configuration recovery only. No NAND writes, independent Safe Mode, save restore or application rollback.",
                    Option::Nul(), {"Foundation"}, {0});
    }
    gui().addPage(page);
}
}
