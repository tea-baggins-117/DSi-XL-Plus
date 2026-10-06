// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsi_xl_plus/xlplus_runtime.h"
#include "common/systemdetails.h"
#include "common/flashcard.h"
#include <nds.h>

namespace dsi_xl_plus {
namespace { RuntimeContext context; }
static_assert(sizeof(RuntimeContext) <= 512, "Foundation persistent state budget exceeded");
void initialize() {
    if (context.initialized) return;
    context.initialized = true;
    const bool fromSd = sys().isRunFromSD();
    const bool available = fromSd ? sdFound() : flashcardFound();
    context.device = available ? (fromSd ? Device::Sd : Device::Flashcard) : Device::Unavailable;
    // This is an execution-mode gate, not automatic DSi XL model detection.
    context.supportedMode = available && fromSd && isDSiMode();
    if (available) context.recovery = loadConfiguration(ndsStorage(context.device));
    else {
        context.recovery.active.code = Code::IoError;
        context.recovery.hadIoError = true;
    }
}
const RuntimeContext& runtime() { return context; }
void settingsCheckpoint() {
    if (!context.initialized) return;
    settingsReady(ndsStorage(context.device), context.recovery, context.supportedMode);
}
}
