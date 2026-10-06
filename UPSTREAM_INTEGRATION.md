# DSi XL+ upstream integration record

Development branch: `dsi-xl-plus/main`. Upstream: [DS-Homebrew/TWiLightMenu](https://github.com/DS-Homebrew/TWiLightMenu). Fork: `tea-baggins-117/DSi-XL-Plus`.

The sole starting source is upstream baseline `97b22fe2cc282cee5bae7c12e0db54c1aa111dfb`, tree `137506985c198fb32bb6c1d7e7ddd0b2be5ec552`. DLDI stays at `4e3fba1f4a96dc0ee53b97cecbfe497633b7dfd5`. The user approved the Phase 1 architecture on 6 October 2026 and clarified that no separate starter implementation exists.

Phase 0's untouched build passed with exit 0. Its archive remains unchanged at `/home/tea_baggins/Projects/DSiXLPlus-Baselines/20261006T163517Z-97b22fe-sdoIBT`. Phase 1 uses independent source/build/evidence directories. Hardware smoke testing remains **PENDING**, including the untouched baseline.

## Toolchain and input identity

- Repository Dockerfile; `devkitpro/devkitarm:20241104`.
- Base digest `sha256:a998edf6b06416b5c053edbcd879abfa22b1b88e9cd3f267f5c4ee9fec71a93a`.
- Verified Phase 0 derived image ID `sha256:e24dec0892a00f52c3ceb16fbd2711a63a754467a6c13057bcda82a57101cd2b`.
- devkitARM r65 / GCC 14.2.0; libnds 1.8.3-1; libfat-nds 1.1.5-1.
- Manual archive SHA-256 `72b75b98600ce78c3995803f1aa87b9a94384778d7bd6b5d20520ff62f826614`.
- Original recorded Phase 0 package SHA-256 `78f3165be31fef3d5e828a1decf293a25d57bdb3e261d1ea99484c486817a0a9`. Its bytes were not uploaded for independent verification.

The build helper requires the same base digest and manual input. It uses the original Dockerfile first; only a matching obsolete bullseye-security apt failure permits the existing upstream CI workaround in a separate build-context file. No source or library compatibility patches are made to accommodate the native libnds-v2 environment.

## Eight modified upstream files

| File | Change |
| --- | --- |
| `README.md` | Fork identity and docs links; upstream content retained. |
| `.github/workflows/nightly.yml` | Add `dsi-xl-plus/**` to both branch lists. |
| `title/arm9/Makefile` | Include opt-in foundation build fragment. |
| `romsel_dsimenutheme/arm9/Makefile` | Same. |
| `settings/arm9/Makefile` | Same, with settings-only adapter source. |
| `title/arm9/source/main.cpp` | Guarded read-only foundation initialization. |
| `romsel_dsimenutheme/arm9/source/main.cpp` | Guarded read-only initialization before theme resources. |
| `settings/arm9/source/main.cpp` | Guarded initialization, read-only page, once-only post-draw maintenance checkpoint. |

All new runtime code lives under `dsi-xl-plus/`. The additional foundation workflow is a new file. No edits to ARM7, shared main, booters, launch paths, per-game settings, renderer, copy/INI helpers, NAND/SDMMC drivers, root packaging, Dockerfile, release workflow or submodule pin.

## Merge and review procedure

1. Preserve local work; confirm development branch and clean source before merging.
2. Review upstream changes around each of the eight files, especially initialization order and settings page contracts.
3. Keep core file basenames unique (`xlplus_`), explicit source directories and a compile-off path with no XL+ objects.
4. Build baseline/off/on in separate fresh checkouts. Compare output inventory, warning messages, ELF sizes/maps and new code paths. Git-derived version/debug metadata means byte equality is not promised.
5. Run host fault tests and relevant hardware regressions before accepting an upstream update. A small merge diff is not proof of semantic compatibility.

## Approved-plan clarifications

- No starter-source gate remains; the user explicitly resolved it.
- Oversized and unsupported-schema files are preserved in place. An oversized file cannot receive a complete bounded content stamp, so automatic quarantine is deliberately skipped.
- Content stamps are bounded FNV-1a change detectors, not cryptographic authenticity checks. Staged copies also undergo byte-for-byte comparison and reparsing.
- Config maintenance is limited by execution mode (`isDSiMode`, launcher running from available SD). This does not identify DSi XL versus every other compatible model; all untested hardware routes remain unverified.
- The session has no Docker daemon. The same pinned Docker configuration is used by the new GitHub workflow and the host helper. Build acceptance must be established from their actual exit codes, not host tests alone.
- The existing upstream `build/` ignore rule also matches `dsi-xl-plus/build/xlplus.mk`; track that one approved file explicitly rather than changing a ninth upstream file.

See [testing](dsi-xl-plus/docs/TESTING.md) for acceptance criteria and evidence interpretation.
