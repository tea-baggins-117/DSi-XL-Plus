# Development installation status

No complete DSi XL+ installable release or automatic installer is provided in Phase 1. `make package` produces a mixed-platform build tree; upstream release preparation adds dependencies and platform-specific arrangements. Do not copy the whole tree onto an SD card.

Preserve the verified Phase 0 archive and back up the existing SD contents before any separately arranged hardware test. Test the untouched baseline first on the actual DSi XL, then compare the foundation build on the same known setup. Hardware smoke testing is pending.

Foundation-off builds need no XL+ files. Foundation-on builds also boot through the existing launcher flow with a missing XL+ directory/config: compiled defaults disable optional behavior. For deliberate testing, the documented schema can be placed under the correct launcher-device root; no automatic file creation, NAND installation or formatting is performed by the build helper.
