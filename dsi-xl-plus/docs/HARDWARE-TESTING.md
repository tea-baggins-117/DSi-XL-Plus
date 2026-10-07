# DSi XL+ — beginner hardware-test guide

**Hardware smoke testing: PENDING. Phase 2 has not started.** These packages are for an actual DSi XL using its SD-card slot and an already-working, standard TWiLight Menu++ SD installation. They are not first-time installers or full TWiLight updates.

The purpose is to compare the untouched launcher, the foundation compiled out, and the foundation compiled in on the same card/dependencies. No package installs Unlaunch, changes NAND, formats a card, changes autoboot settings, or contains an automatic installer.

## Choose the correct package

| Order | ZIP | What it does |
| --- | --- | --- |
| 1 | `01-PHASE0-BASELINE-97b22fe-SD.zip` | Untouched upstream Phase 0 source, commit `97b22fe2cc282cee5bae7c12e0db54c1aa111dfb`; no XL+ code. |
| 2 | `02-PHASE1-FOUNDATION-OFF-2d72ffc-SD.zip` | Phase 1 commit `2d72ffc09e130ee72eae934f07d0b8923b9f9cc1`, built with `XLPLUS=0`. No XL+ settings page or configuration handling. |
| 3 | `03-PHASE1-FOUNDATION-ON-2d72ffc-SD.zip` | The same Phase 1 commit, built with `XLPLUS=1`. Includes an enabled test configuration, copied in a separate second pass below. |

All binaries are extracted unchanged from accepted build run [37564486683](https://github.com/tea-baggins-117/DSi-XL-Plus/actions/runs/37564486683). The packaging tool checks the original archive SHA-256 and every selected file's size/hash. It does not rebuild the programs. Package 1 is the accepted **CI rebuild of unchanged Phase 0 source**, not a byte-for-byte copy of the earlier Mint archive. The earlier record found matching debug ELF hashes but different packed loader hashes; that distinction remains recorded. Your archived Mint Phase 0 build stays untouched.

Keep your current nds-bootstrap, add-ons, fonts, color filters and theme assets unchanged for all three tests. Record their existing versions. These packages do not install those dependencies; game compatibility observations are tied to the existing working SD setup. If TWiLight does not already work, or `BOOT.NDS` on your card belongs to another application, stop and get the standard installation/launch route identified before using these packages. Do not overwrite an unrelated boot file.

## Before copying anything — do this once

1. Confirm that the current card boots TWiLight Menu++ successfully on your DSi XL. Note its version, nds-bootstrap version if displayed, and your launch method (for example, Memory Pit or an existing Unlaunch entry). Keep that same launch method throughout the comparison. Do not install or reconfigure a bootloader.
2. Fully power off the DSi XL before removing its SD card. Put the card in your laptop/card reader.
3. In Linux Mint's File Manager, open the SD card from the left sidebar. Its **root** is the first level where you normally see `BOOT.NDS`, `_nds`, and perhaps `private` and `roms`. It is not your laptop's system folder. The notation `sd:/` below means this SD-card root; do not create a folder literally named `sd:`.
4. Make a new folder on the laptop named `DSiXLPlus-before-hardware-tests`. Copy the entire card's contents into it, including hidden files (Ctrl+H shows them). Copy, do not move. Wait for copying to finish and inspect the backup. Keep your existing archived Phase 0 build separate and unchanged.
5. Prefer a spare test card containing a complete copy of the already-working card. Keep the original card untouched. Do not erase or format your working card for this test.
6. If the test card already has `_nds/DSiXLPlus`, preserve it in the laptop backup, then rename that card folder to an unused name such as `DSiXLPlus-before-hardware-test`. If that name already exists, choose another unused name and record it. This isolates old XL+ configs and known-good copies without deleting them. Leave the renamed folder alone until rollback.
7. Download and extract each ZIP into its **own named folder on the laptop**. Never combine all three extracts. Each SD ZIP contains only `BOOT.NDS` and an `_nds` subtree. The guide, CSV manifests and hash records are separate laptop-side documents; do not copy them onto the SD card.
8. Use disposable copies of a known-working homebrew/game and its save when testing software launch. Prefer the spare card. Do not conduct new-save or recovery experiments against the only copy of a valuable save. Normal TWiLight/game use can update settings and saves even though the installation overlay does not contain those files.

## Exactly which files are replaced?

Each package replaces the same **nine launcher files**, relative to the SD-card root:

| Destination | Role |
| --- | --- |
| `BOOT.NDS` | Existing standard SD launch entry |
| `_nds/TWiLightMenu/main.srldr` | Title/startup launcher |
| `_nds/TWiLightMenu/dsimenu.srldr` | DSi/3DS/Saturn/Homebrew Launcher UI selector |
| `_nds/TWiLightMenu/settings.srldr` | TWiLight settings |
| `_nds/TWiLightMenu/mainmenu.srldr` | DS Classic Menu |
| `_nds/TWiLightMenu/akmenu.srldr` | Wood UI selector |
| `_nds/TWiLightMenu/r4menu.srldr` | R4/Game Boy Color UI selector |
| `_nds/TWiLightMenu/manual.srldr` | Built-in manual |
| `_nds/TWiLightMenu/slot1launch.srldr` | Slot-1 launch helper |

Foundation-on additionally supplies `_nds/DSiXLPlus/settings.ini` for its second test pass. That is **not** `_nds/TWiLightMenu/settings.ini`.

Each package has a matching `*-MANIFEST.csv` listing every destination, byte count, SHA-256, copy action and source archive entry. Its `*-SHA256SUMS.txt` lists those same SD paths for optional read-only verification from the SD root. `PACKAGE-SHA256SUMS.txt` checks the downloadable ZIPs instead. Foundation-on's SD checksum list includes the config: before the second pass that one file should intentionally be absent.

ROMs, saves, NAND backups, Memory Pit/private files, the existing TWiLight `settings.ini`, per-game settings, nds-bootstrap, hiyaCFW/SDNAND `title` files, custom themes, and unrelated SD contents are outside these overlays. No flashcard/3DS installers, debug ELFs, Unlaunch assets, ntrboot payloads, GBA/emulator binaries or multimedia add-ons are included.

## Package 1 — untouched Phase 0 baseline

1. Open the extracted `01-PHASE0-BASELINE-97b22fe-SD` folder on the laptop. Confirm its `BOOT.NDS` is beside its `_nds` folder. Keep the backup from the preparation steps.
2. Copy **only** this `BOOT.NDS` into the SD-card root. Approve replacing the existing TWiLight `BOOT.NDS` file.
3. Open the package's `_nds/TWiLightMenu` folder. Select its **eight `.srldr` files** and copy them into the SD card's existing `_nds/TWiLightMenu` folder. Replace matching files only. Do not delete or replace the `_nds` or `TWiLightMenu` directory itself. Do not select other folders on the card.
4. Wait for all copying to finish. Use the File Manager's eject/unmount button, wait until it completes, then remove the card and put it back in the fully powered-off DSi XL.
5. Start TWiLight using the same working method as before. Run the smoke checklist below and write results in the baseline row of `HARDWARE-RESULTS.csv`. No XL+ settings page is expected.
6. **If the baseline fails, stop here.** Record the screen/error and restore your pre-test launcher files using the rollback steps. Do not proceed to foundation-off/on to work around a failing control build.

## Package 2 — Phase 1 foundation OFF

1. Continue only after the untouched baseline works. Fully power off, remove the SD card, and connect it to the laptop.
2. Open `02-PHASE1-FOUNDATION-OFF-2d72ffc-SD`. Copy its `BOOT.NDS` to the SD root, replacing the matching TWiLight file.
3. Copy its eight files from `_nds/TWiLightMenu` into the same existing SD folder, replacing matching files only. Leave all surrounding folders and user files alone.
4. Do not add an XL+ configuration file. Foundation-off has XL+ compiled out; it ignores XL+ configs even if one were present. Keep the isolated pre-test XL+ folder preserved.
5. Eject safely, put the card back, and repeat the same smoke checklist with the same launch method, dependencies and disposable software copies. Record the foundation-off row. The XL+ page should still be absent. The upstream version suffix may differ from the baseline because this build identifies the Phase 1 commit.
6. If behavior regresses, record it and reinstall Package 1's nine files. Do not continue until the discrepancy is understood.

## Package 3 — Phase 1 foundation ON

**First pass: missing configuration / disabled defaults**

1. After foundation-off passes, power off and connect the card to the laptop. Open `03-PHASE1-FOUNDATION-ON-2d72ffc-SD`.
2. Copy its `BOOT.NDS` and its eight `_nds/TWiLightMenu/*.srldr` files exactly as for the previous packages. **Do not copy its `_nds/DSiXLPlus` folder yet.** Your earlier XL+ folder should still be preserved under its temporary name, so there should be no active SD folder named `_nds/DSiXLPlus`.
3. Eject safely, boot normally and repeat the smoke checklist. Open TWiLight settings: on the usual DSi UI, SELECT leads to the DS Classic Menu and the DS icon opens settings; alternatively hold SELECT while launching TWiLight. Keep your existing bootloader method unchanged.
4. Find the new **DSi XL+** settings page by switching settings pages. Expect version `0.1.0-dev`, upstream baseline `97b22fe`, **Optional features: Disabled**, and Configuration status indicating **Compiled defaults** / active file not present. There should be no startup error dialog caused by the missing XL+ config. Record the missing-config row. If the new page is absent, stop and check that the foundation-on `settings.srldr` was copied.

**Second pass: enabled test configuration**

5. Fully power off and reconnect the SD card to the laptop. Copy the package's `_nds/DSiXLPlus` folder into the card's existing `_nds` folder. Since the earlier XL+ folder was preserved under another name, this should create a fresh `DSiXLPlus` folder. If the computer unexpectedly asks to overwrite existing XL+ files, cancel and preserve those files first.
6. The final on-console path must be **`sd:/_nds/DSiXLPlus/settings.ini`**. On the laptop that means `SD card → _nds → DSiXLPlus → settings.ini`. Use the supplied file; no manual editing is required. It contains:

   ```ini
   [DSiXLPlus]
   schema=1
   enabled=1
   diagnostics=1
   ```

7. Eject safely, boot again, and open the XL+ page. On the intended DSi-mode SD route, expect **Optional features: Requested**, Configuration source **Active configuration**, and diagnostics indicating **SD**. This request does not implement or activate future dashboard/library/save features. If it stays Disabled despite this file, record the launch route and status rather than bypassing the capability checks.
8. Select **Configuration status** and press A to view the current maintenance result. Settings may create `settings.lkg.ini` in the new XL+ folder after its first normal draw. A later visit with unchanged valid config should report that the known-good copy is unchanged. If storage is read-only/full or another problem prevents maintenance, preserve the status/files and report it. Do not delete arbitrary files to make a warning disappear.
9. Repeat the same smoke checklist and record the enabled-config row. Preserve the new XL+ folder with the test results when finished. No visual redesign is expected in Phase 1.

To repeat the missing-config test later, power off and preserve/rename the **whole test** `DSiXLPlus` folder to an unused name, then boot with no active folder of that name. This keeps the generated LKG/staging/quarantine evidence intact. These tests do not include deliberate power loss, filesystem corruption or NAND recovery.

## Short smoke checklist — repeat for every test row

1. Cold boot and then restart: does the launcher reach the usual screen without new hangs or errors?
2. Navigate with the D-pad/buttons and touch screen. Open/close settings and the built-in manual.
3. Check the existing original UIs you normally use. Use the built-in/default DSi UI for the first controlled comparison; record a custom theme if testing it separately. Keep settings consistent between variants.
4. Launch a disposable copy of a known-working homebrew and, if available, an owner-supplied DS game with a copied save. Record what happens. Use the software's normal exit method or power off after it is idle; never remove power while saving. A game without a return-to-menu feature is not automatically a failed test.
5. Record variant, boot method, settings/config status, software tested and any symptoms. A direct-selector launch does not exercise the title path, so record that limitation. Mark unperformed tests **NOT TESTED**, not passed. Actual free RAM, stack headroom and performance have not yet been measured.

If something fails, stop that test, note the exact behavior, and use the file rollback below. Do not enter NAND/Unlaunch install/uninstall options, change autoboot configuration, format the card or run a system updater as part of this comparison.

## Roll back to the untouched baseline

1. Fully power off, remove the SD card and connect it to the laptop.
2. Copy Package 1's `BOOT.NDS` and eight `.srldr` files back to their exact paths, replacing those nine files only. This restores the untouched **baseline launcher build**; existing nds-bootstrap, assets and user settings remain as they were on the test card.
3. Preserve the test `_nds/DSiXLPlus` folder by copying it to the laptop's results folder, then move it off the card. If you renamed a pre-existing XL+ folder during preparation, rename that original folder back to `DSiXLPlus`. If there was no original XL+ folder, leave that active path absent. Never replace the entire `_nds` directory.
4. Safely eject and retest the baseline. Baseline has no XL+ code; a runtime `enabled=0` setting alone would not be the same rollback as replacing the foundation-on binaries.

## Return to your exact pre-test official launcher instead

1. Use the laptop backup made **before Package 1**, not a newer build or a random release download.
2. Restore its `BOOT.NDS` and the eight listed `.srldr` files to the matching SD paths. If any listed file did not exist before testing, move only that newly-added file off the card instead of leaving it behind. Preserve test results first.
3. Restore the original XL+ folder arrangement as described above. If you deliberately changed TWiLight settings for testing and want them back, restore just the backed-up `_nds/TWiLightMenu/settings.ini` after preserving the test version. Do not overwrite ROMs/saves with an older full-card backup merely to restore launcher files.
4. Eject and boot using your original method. If a spare test card was used, the untouched original card remains the simplest return to your original setup.

Package 1 restores the recorded Phase 0 launcher version, which may differ from the official release previously installed on your card. Your pre-test backup is what restores that exact earlier installation's launcher files. Neither route performs NAND operations.

## Evidence and upstream references

`PACKAGE-PROVENANCE.json` records the accepted archive hashes, binary commits, every selected file and each final test ZIP hash. The companion manifests include the configuration path only for foundation-on. Package bytes were verified; **hardware smoke testing remains pending until you record actual results**. No Phase 2 work is included.

The SD-root arrangement and settings shortcut were checked against the official DS-Homebrew [DSi installation](https://wiki.ds-homebrew.com/twilightmenu/installing-dsi), [DSi updating](https://wiki.ds-homebrew.com/twilightmenu/updating-dsi) and [settings FAQ](https://wiki.ds-homebrew.com/twilightmenu/faq?faq=how-do-i-access-twilight-menu-settings) documentation. Those full-install instructions are background; for these tests follow the narrower nine-file copy list above. TWiLight Menu++ remains attributed to DS-Homebrew; source for the baseline and Phase 1 binaries is available at the exact commits in the project repository.

Exact corresponding source: [unchanged baseline](https://github.com/DS-Homebrew/TWiLightMenu/tree/97b22fe2cc282cee5bae7c12e0db54c1aa111dfb), [Phase 1 foundation](https://github.com/tea-baggins-117/DSi-XL-Plus/tree/2d72ffc09e130ee72eae934f07d0b8923b9f9cc1). The companion documents preserve the upstream license; original source notices remain in the repository.
