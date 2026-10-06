# Configuration recovery foundations

Recovery in Phase 1 covers XL+ configuration only. Dedicated Safe Mode, save restore and application rollback are not implemented. Hardware smoke testing, including actual FAT power-interruption behavior, remains pending.

Startup loads policy read-only. On the first normal settings draw, an idempotent checkpoint may perform maintenance if running in DSi mode from available SD, the existing XL+ root is a directory, and no prior configuration I/O failure occurred. Other routes stay read-only. Execution-mode detection is not DSi XL model detection.

An unchanged valid active configuration can be copied as last-known-good after that checkpoint. This certifies only successful Phase 1 configuration initialization, not gameplay or future assets. Identical active/LKG bytes generate no new writes. Default/fallback configuration is never promoted over LKG.

Writes are restricted to fixed owned config names. The writer exclusively creates `settings.lkg.tmp`, checks partial/zero writes, `fflush`, `fsync`, `fclose`, reopens/reparses, and compares bytes with the stable active file. An existing temporary file is preserved and blocks maintenance. A valid old LKG is moved to `settings.lkg.prev.ini` before the verified stage is promoted; an older predecessor is removed only while both current LKG and verified stage exist. A lone predecessor is retained. Every operation and final reread is checked.

Malformed active files can be renamed to the first unused `settings.bad.0.ini` through `.3.ini`. Malformed LKG files use `settings.lkg.bad.0.ini` through `.3.ini` before replacement. Full pools, read-only/full storage and failures leave originals in place. No existing quarantine file is overwritten. Unsupported schemas, incomplete oversized-file stamps and I/O failures do not trigger quarantine.

The content stamp combines byte length and FNV-1a as a lightweight change detector; it is not a security hash. The staged copy additionally receives complete bounded byte comparison. The console must have exclusive use of its SD filesystem during maintenance. If an unexplained stage or full quarantine pool blocks recovery, preserve those files on the host and inspect them before removing any owned temporary/evidence file. No automatic broad deletion is provided.

Pinned libfat v1.1.5 at `fef8efe371e97b5b2281c6a8f6e4ba6c46f7dd12` was inspected: `_FAT_fsync_r` and writable close report sync failures; `_FAT_rename_r` rejects existing destinations. These checks improve error detection but do not make multi-file FAT updates atomic or prove media durability. A power loss can still damage a filesystem.

Keep a complete external copy of the verified Phase 0 build and user data. XL+ fallback cannot repair failed shared FAT initialization, unreadable media, a damaged executable or a bootloader. Do not restore a mixed-platform package blindly over a working SD card. No NAND writes, formatting, Unlaunch installation or user-save operations are introduced.
