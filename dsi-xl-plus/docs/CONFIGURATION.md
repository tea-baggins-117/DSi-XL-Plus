# Configuration schema 1

Configuration belongs to the launcher device: `sd:/_nds/DSiXLPlus/settings.ini` when `sys().isRunFromSD()` is true and SD is available, otherwise the equivalent `fat:` root when the flashcard is available. No cross-volume fallback, current-directory dependency or browsed-ROM-device selection is used. An unavailable volume yields disabled defaults and a storage-error status.

```ini
[DSiXLPlus]
schema=1
enabled=0
diagnostics=0
```

`schema` is required. `enabled` and `diagnostics` default to false and accept exactly `0` or `1`. Names are case-sensitive. Section/key identifiers use letters, digits, underscore, hyphen or dot. Optional diagnostics add read-only detail; they do not write logs. The foundation page/recovery maintenance remains available in a compiled-in build even when optional features are disabled. There is no on-console configuration editor in Phase 1.

Bounds: 8,192 bytes/file, 128 logical lines, 255 bytes/line, 32-byte keys/section names and 64-byte values. Limits are enforced while reading. LF/CRLF, final line without newline and a leading UTF-8 BOM are supported. Leading/trailing space/tab is trimmed. Full-line `#` and `;` comments are accepted; inline comments are not interpreted. NUL, disallowed controls, bad syntax and duplicate known fields are rejected. Unknown bounded keys/sections are ignored under schema 1 and preserved byte-for-byte in known-good copies. A recognizable different numeric schema is preserved without migration or automatic quarantine.

| Active file | Result |
| --- | --- |
| Valid | Use its fields; apply compile/runtime gates. A disabled active file overrides any enabled LKG. |
| Missing | Disabled compiled defaults; no directory/file creation and no LKG resurrection. |
| Malformed/oversized | Try valid `settings.lkg.ini`, then `settings.lkg.prev.ini`, then defaults; optional behavior stays disabled. |
| Unsupported schema | Same conservative fallback; leave original untouched. |
| I/O error / non-regular path | Report error, try readable same-volume known-good candidates, otherwise defaults; no maintenance. |

Only active, LKG and predecessor participate in loading. Staged and quarantined files never become implicit configuration candidates. Files too large for a complete bounded stamp stay in place. Deleting the active config remains a reliable disable action.

`settings.example.ini` documents the schema but is not auto-installed or required to boot. Compiled defaults are authoritative. XL+ does not modify TWiLight Menu++ settings, nds-bootstrap INIs or per-game settings.
