# Foundation verification

**Hardware smoke testing: PENDING**, including the untouched Phase 0 baseline. Build acceptance requires actual pinned-Docker `make package` exit 0; host tests do not substitute for it.

Local host verification on 6 October 2026 passed with GNU C++ 13.3, C++17, no RTTI/exceptions and `-Wall -Wextra -Werror`:

- Config syntax/limits/defaults, one-byte reads, every read/close error and 2,000 deterministic arbitrary byte inputs.
- Missing-active disabled behavior, active/LKG/predecessor/default ordering, unsupported-schema preservation, runtime gates and checkpoint idempotence.
- All 70 operation-failure points and seven mutation/interruption boundaries in a normal LKG rotation; partial and zero writes, corruption, read-only storage, occupied stage, full quarantine pools, invalid/newer LKG and predecessor-only recovery.
- Actual stdio/POSIX backend in a disposable host directory: missing root produces no files, exclusive staging, verified promotion, unchanged zero-write path and forbidden target operations.
- Compile gate on and off for recovery policy.
- AddressSanitizer and UndefinedBehaviorSanitizer. The local environment blocks LeakSanitizer process inspection, so that local run used `ASAN_OPTIONS=detect_leaks=0`; CI attempts the standard sanitizer configuration. Fake-storage handle counts independently check all opened handles are closed.

These are deterministic filesystem-operation simulations, not proof of FAT sector/power-loss behavior. The real adapter host test is not an ARM or libfat runtime test.

## Console build evidence

The `DSi XL+ foundation checks` workflow runs host tests followed by independent baseline/off/on clean package builds using the existing Dockerfile and verified base/manual hashes. It preserves logs, exit codes, image/package provenance, inventories, symbol/size reports, maps, compiled packages and a comparison report. The original nightly branch filter is also extended to include the development branch.

At this source commit's preparation, console build acceptance is pending the workflow result. Associate evidence with the exact source commit, not a moving branch name. Review the recorded `build.exit-code`, complete logs, artifact inventory, newly introduced warning messages and actual ELF growth before declaring Phase 1 complete. Existing upstream warnings are retained rather than suppressed. A successful compiler result does not certify those warnings harmless.

## Hardware gate before Phase 2

1. Test the preserved untouched baseline on actual DSi XL: cold/warm boot, settings, original themes, buttons/touch, representative owner-supplied homebrew/DS launch and return.
2. Repeat on foundation-off and foundation-on builds using the same setup. Check startup keys/autorun, direct selector entry and settings page normal/RTL/Macro Mode layouts.
3. Check missing/disabled/enabled/malformed/newer configs, LKG/predecessor fallback, read-only/low-space cases on disposable test data, repeated settings visits with no repeated writes, and truthful maintenance failures.
4. Record startup timing, free heap/stack behavior and the three affected ELF/map size changes. Core working-buffer target is 4 KiB and persistent core state is at most 512 bytes; UI/stdio memory and executable growth are additional measured costs.
5. Label DS-mode, flashcard, DSiWare and 3DS routes untested unless actually exercised. Execution mode is not automatic console-model identification.

Do not intentionally interrupt writes to the user's only SD card or saves. Do not advance to Phase 2 UI work until the foundation results and hardware comparison have been reviewed.
