// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#ifndef DSIXLPLUS_FOUNDATION
#define DSIXLPLUS_FOUNDATION 0
#endif
#if DSIXLPLUS_FOUNDATION != 0 && DSIXLPLUS_FOUNDATION != 1
#error "DSIXLPLUS_FOUNDATION must be 0 or 1"
#endif

namespace dsi_xl_plus {
inline constexpr bool kFoundationBuilt = DSIXLPLUS_FOUNDATION == 1;
// Availability, not user preferences. These features have no implementation yet.
inline constexpr bool kDashboardAvailable = false;
inline constexpr bool kLibraryAvailable = false;
inline constexpr bool kProfilesAvailable = false;
inline constexpr bool kSaveManagerAvailable = false;
inline constexpr bool kSafeModeAvailable = false;
}
