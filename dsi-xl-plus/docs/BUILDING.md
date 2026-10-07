# Foundation builds

Work on `dsi-xl-plus/main`, never `master`. Preserve the existing Phase 0 directory. Use the repository's Dockerfile and `devkitpro/devkitarm:20241104`; do not fix native libnds-v2 errors by changing source or downgrading isolated libraries.

Host tests need a C++17 compiler and Bash:

```sh
bash dsi-xl-plus/tests/run-host-tests.sh
SANITIZE=1 bash dsi-xl-plus/tests/run-host-tests.sh
```

Commit or preserve changes before clean console checks. From a clean development checkout:

```sh
bash dsi-xl-plus/tools/run-foundation-checks.sh
```

The helper requires Docker daemon access without sudo, Git, curl, 7z, Python 3.11+, tar and sha256sum. It creates a unique directory under `~/Projects/DSiXLPlus-Phase1-Builds/`, builds the existing Dockerfile, validates the recorded base digest and manual input, and creates three separate fresh build copies: untouched baseline, foundation off, foundation on. Compilation uses serial `make package` with `XLPLUS=0` or `XLPLUS=1`. Optional argument: `baseline`, `off`, or `on` instead of the default `all`.

`XLPLUS_OUTPUT_PARENT` can select a separate evidence parent. `XLPLUS_MANUAL_ARCHIVE` can reference a preserved manual archive read-only; its hash must match Phase 0. The helper never runs inside the Phase 0 archive, installs to SD, changes the submodule pin, or overwrites an old build directory.

Upstream version generation requires reachable tags. The fork may have no tags, even with full history. In each isolated build copy, the helper obtains the official `v27.24.1` tag, verifies its object SHA `68d04c1a621a8d330e7233efcfcb93c94b30a3a6` and baseline ancestry, and records the resulting version. If the copy is shallow, it first obtains the baseline's full history from official upstream. The development checkout and archived baseline remain unchanged. The workflow already checks out full history, so it only needs the tag fetch. `make package XLPLUS=1` can also be run directly inside the pinned environment with this version metadata available, but use a fresh checkout when changing the flag: Make does not reliably detect compiler-flag changes in existing objects.

Evidence includes commit/submodule identity, source diff, image metadata, toolchain packages, full logs, build exit status, package hashes/inventory, three ELF size/symbol reports and linker maps. Each variant preserves its compiled package separately. CI additionally compares output inventories, warnings and ELF sizes; inspect new warnings before accepting a build.

`dsi-xl-plus/build/xlplus.mk` must remain tracked even though upstream ignores directories named `build`. It is source, not generated output. No new Dockerfile is required.
