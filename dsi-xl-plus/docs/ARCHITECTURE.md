# Foundation architecture

`dsi_xl_plus` is isolated from upstream shared sources. `src/core` contains a bounded parser, deterministic recovery policy and checked config persistence against an injectable fixed-file storage interface. It has no libnds/UI dependency or parser heap allocation. `src/nds` reads existing device/mode state and implements file operations using the pinned libfat/newlib interfaces. `src/nds/settings` alone depends on upstream settings page types.

Each of `main.srldr`, `dsimenu.srldr` and `settings.srldr` initializes its own context after upstream filesystem setup. No cross-chainload globals, magic RAM flags, remounts or new launch protocol are used. Title and selector are read-only. Settings calls an idempotent recovery checkpoint after its normal first draw.

The three component Makefiles include one opt-in fragment. It resolves from the owning ARM9 Makefile even inside its recursive build directory. Explicit relative source directories work with upstream's VPATH construction. Unique `xlplus_` basenames avoid its flattened-object naming collisions. Settings UI adapter code is never compiled into title or selector.

`XLPLUS=0` excludes new objects, includes, calls and page. `XLPLUS=1` builds the foundation; runtime `enabled=0` masks optional features by default. Only an active valid config, supported execution mode and built feature can permit optional behavior. Recovery always masks it off. All future feature-availability constants remain false.

The information page reuses `SettingsPage` and inert `Option::Nul` rows with one value each. Its recovery detail is generated when opened, so the displayed maintenance result reflects the completed checkpoint. Existing upstream footer, bootstrap version, inputs and settings persistence are unchanged. Page strings are currently English development text; RTL and Macro Mode layout are pending hardware validation.

The core persistent-state budget is 512 bytes; parser/storage buffers target a combined 4 KiB maximum working set. Stdio/UI allocations and executable growth must also be measured separately. No texture, ROM index, background scan or periodic logger is added.
