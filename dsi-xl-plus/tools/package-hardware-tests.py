#!/usr/bin/env python3
"""Extract DSi launcher test overlays from the already accepted CI archives.

No compilation, device discovery, SD writes, NAND operations, or network access.
The input archive and every selected binary must match recorded acceptance hashes.
"""
# SPDX-License-Identifier: GPL-3.0-or-later
import argparse
import csv
import hashlib
import io
import json
from pathlib import Path
import tarfile
import zipfile

ACCEPTED_RUN = 37564486683
PHASE1_COMMIT = "2d72ffc09e130ee72eae934f07d0b8923b9f9cc1"
DLDI = "4e3fba1f4a96dc0ee53b97cecbfe497633b7dfd5"
INPUTS = {'baseline': {'source_commit': '97b22fe2cc282cee5bae7c12e0db54c1aa111dfb',
              'archive_sha256': '540a89384fdf3c0920686e0f6af0ca3e7f818814e3f530f94fd8507e0b2916d8',
              'files': {'DSi&3DS - SD card users/BOOT.NDS': {'bytes': 309696,
                                                             'sha256': 'db754a2f2bdd91c65a8e3d773d84c6ce6a3364ee4fff2810cf07864f4c8d1fce',
                                                             'destination': 'BOOT.NDS'},
                        '_nds/TWiLightMenu/akmenu.srldr': {'bytes': 1277440,
                                                           'sha256': 'fe5a1a32554f5e4c22aa480468f884849a2b843cd166d436fb46db63b26b65f3',
                                                           'destination': '_nds/TWiLightMenu/akmenu.srldr'},
                        '_nds/TWiLightMenu/dsimenu.srldr': {'bytes': 4591104,
                                                            'sha256': '8f12fac0416e577506d939986359a0cc0054062391bb15106a95b0f294f05922',
                                                            'destination': '_nds/TWiLightMenu/dsimenu.srldr'},
                        '_nds/TWiLightMenu/main.srldr': {'bytes': 5658112,
                                                         'sha256': '4213d1d2d621995b5fb391e00a8e9e587830b37a78e4eae08f58225868fb6d67',
                                                         'destination': '_nds/TWiLightMenu/main.srldr'},
                        '_nds/TWiLightMenu/mainmenu.srldr': {'bytes': 1298944,
                                                             'sha256': 'c644b5be8a2286e439fd184dc8cf624f72421dc413463176c2334bb3efbae3a8',
                                                             'destination': '_nds/TWiLightMenu/mainmenu.srldr'},
                        '_nds/TWiLightMenu/manual.srldr': {'bytes': 20419072,
                                                           'sha256': '0c7f150f1ac5c826b71a08495c2fb739b21b4c845089c11a7a63377100c83b60',
                                                           'destination': '_nds/TWiLightMenu/manual.srldr'},
                        '_nds/TWiLightMenu/r4menu.srldr': {'bytes': 2924032,
                                                           'sha256': 'f9c6956e3e436f4929c18962ee80eab88ca3da4491a6a682c2d81389371243a5',
                                                           'destination': '_nds/TWiLightMenu/r4menu.srldr'},
                        '_nds/TWiLightMenu/settings.srldr': {'bytes': 1864192,
                                                             'sha256': '949c711faf8e91afd1a469d5b95946ee1fcd827dce9a9abe676bdad02fcd2b84',
                                                             'destination': '_nds/TWiLightMenu/settings.srldr'},
                        '_nds/TWiLightMenu/slot1launch.srldr': {'bytes': 410112,
                                                                'sha256': 'd9ea3e593d8b7e1e21f1e7d96eef5562371b52f79dd63d21afc592ba9a348e0f',
                                                                'destination': '_nds/TWiLightMenu/slot1launch.srldr'}}},
 'off': {'source_commit': '2d72ffc09e130ee72eae934f07d0b8923b9f9cc1',
         'archive_sha256': 'd8135c2aa0a0a030fd18aa31f816ad283de882dda4db0256a92c2703b58e975e',
         'files': {'DSi&3DS - SD card users/BOOT.NDS': {'bytes': 309696,
                                                        'sha256': 'db754a2f2bdd91c65a8e3d773d84c6ce6a3364ee4fff2810cf07864f4c8d1fce',
                                                        'destination': 'BOOT.NDS'},
                   '_nds/TWiLightMenu/akmenu.srldr': {'bytes': 1277440,
                                                      'sha256': 'fe5a1a32554f5e4c22aa480468f884849a2b843cd166d436fb46db63b26b65f3',
                                                      'destination': '_nds/TWiLightMenu/akmenu.srldr'},
                   '_nds/TWiLightMenu/dsimenu.srldr': {'bytes': 4591104,
                                                       'sha256': '8f12fac0416e577506d939986359a0cc0054062391bb15106a95b0f294f05922',
                                                       'destination': '_nds/TWiLightMenu/dsimenu.srldr'},
                   '_nds/TWiLightMenu/main.srldr': {'bytes': 5658112,
                                                    'sha256': '4213d1d2d621995b5fb391e00a8e9e587830b37a78e4eae08f58225868fb6d67',
                                                    'destination': '_nds/TWiLightMenu/main.srldr'},
                   '_nds/TWiLightMenu/mainmenu.srldr': {'bytes': 1298944,
                                                        'sha256': 'c644b5be8a2286e439fd184dc8cf624f72421dc413463176c2334bb3efbae3a8',
                                                        'destination': '_nds/TWiLightMenu/mainmenu.srldr'},
                   '_nds/TWiLightMenu/manual.srldr': {'bytes': 20419072,
                                                      'sha256': '0c7f150f1ac5c826b71a08495c2fb739b21b4c845089c11a7a63377100c83b60',
                                                      'destination': '_nds/TWiLightMenu/manual.srldr'},
                   '_nds/TWiLightMenu/r4menu.srldr': {'bytes': 2924032,
                                                      'sha256': 'f9c6956e3e436f4929c18962ee80eab88ca3da4491a6a682c2d81389371243a5',
                                                      'destination': '_nds/TWiLightMenu/r4menu.srldr'},
                   '_nds/TWiLightMenu/settings.srldr': {'bytes': 1864192,
                                                        'sha256': 'f3f226b597cd14d59109aebbf05606427553ab170a04727dac07fa9a410b9b7a',
                                                        'destination': '_nds/TWiLightMenu/settings.srldr'},
                   '_nds/TWiLightMenu/slot1launch.srldr': {'bytes': 410112,
                                                           'sha256': 'd9ea3e593d8b7e1e21f1e7d96eef5562371b52f79dd63d21afc592ba9a348e0f',
                                                           'destination': '_nds/TWiLightMenu/slot1launch.srldr'}}},
 'on': {'source_commit': '2d72ffc09e130ee72eae934f07d0b8923b9f9cc1',
        'archive_sha256': 'b23cd8d5c5c17f2353db4289ec840e6fece5307a248621da24c21fc445a3c947',
        'files': {'DSi&3DS - SD card users/BOOT.NDS': {'bytes': 309696,
                                                       'sha256': 'db754a2f2bdd91c65a8e3d773d84c6ce6a3364ee4fff2810cf07864f4c8d1fce',
                                                       'destination': 'BOOT.NDS'},
                  '_nds/TWiLightMenu/akmenu.srldr': {'bytes': 1277440,
                                                     'sha256': 'fe5a1a32554f5e4c22aa480468f884849a2b843cd166d436fb46db63b26b65f3',
                                                     'destination': '_nds/TWiLightMenu/akmenu.srldr'},
                  '_nds/TWiLightMenu/dsimenu.srldr': {'bytes': 4598272,
                                                      'sha256': '8ce9803776326b8cbc07016b6912a637b5e862d8f567b7e88439f651e874ceff',
                                                      'destination': '_nds/TWiLightMenu/dsimenu.srldr'},
                  '_nds/TWiLightMenu/main.srldr': {'bytes': 5664256,
                                                   'sha256': '305ec73c1c3aa3317e7917845dbcc28985c549ae356849de302061ce5f3746b9',
                                                   'destination': '_nds/TWiLightMenu/main.srldr'},
                  '_nds/TWiLightMenu/mainmenu.srldr': {'bytes': 1298944,
                                                       'sha256': 'c644b5be8a2286e439fd184dc8cf624f72421dc413463176c2334bb3efbae3a8',
                                                       'destination': '_nds/TWiLightMenu/mainmenu.srldr'},
                  '_nds/TWiLightMenu/manual.srldr': {'bytes': 20419072,
                                                     'sha256': '0c7f150f1ac5c826b71a08495c2fb739b21b4c845089c11a7a63377100c83b60',
                                                     'destination': '_nds/TWiLightMenu/manual.srldr'},
                  '_nds/TWiLightMenu/r4menu.srldr': {'bytes': 2924032,
                                                     'sha256': 'f9c6956e3e436f4929c18962ee80eab88ca3da4491a6a682c2d81389371243a5',
                                                     'destination': '_nds/TWiLightMenu/r4menu.srldr'},
                  '_nds/TWiLightMenu/settings.srldr': {'bytes': 1876480,
                                                       'sha256': '20cd155b15dba33e5a7c9080bdaac3b09d5e3918c33ffcd00e03a50f43fe6ee6',
                                                       'destination': '_nds/TWiLightMenu/settings.srldr'},
                  '_nds/TWiLightMenu/slot1launch.srldr': {'bytes': 410112,
                                                          'sha256': 'd9ea3e593d8b7e1e21f1e7d96eef5562371b52f79dd63d21afc592ba9a348e0f',
                                                          'destination': '_nds/TWiLightMenu/slot1launch.srldr'}}}}
NAMES = {
    "baseline": "01-PHASE0-BASELINE-97b22fe-SD",
    "off": "02-PHASE1-FOUNDATION-OFF-2d72ffc-SD",
    "on": "03-PHASE1-FOUNDATION-ON-2d72ffc-SD",
}
CONFIG_PATH = "_nds/DSiXLPlus/settings.ini"
CONFIG = b"# Phase 1 hardware test only. Future features remain unavailable.\n[DSiXLPlus]\nschema=1\nenabled=1\ndiagnostics=1\n"


def sha_file(path):
    with path.open("rb") as f:
        return hashlib.file_digest(f, "sha256").hexdigest()


def zip_bytes(path, entries):
    with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for name, content in sorted(entries.items()):
            info = zipfile.ZipInfo(name, date_time=(2026, 10, 7, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o100644 << 16
            z.writestr(info, content, compresslevel=9)
    with zipfile.ZipFile(path) as z:
        assert z.testzip() is None
        assert set(z.namelist()) == set(entries)
        for name, content in entries.items():
            assert z.read(name) == content
    assert path.stat().st_size < 32 * 1024 * 1024, "Transfer-sized artifact exceeded 32 MiB"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inputs", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    # Use a new output directory; never overwrite a previous test package set.
    args.output.mkdir(parents=True, exist_ok=False)
    docs = args.output / "instructions-and-manifests"
    docs.mkdir()
    provenance = {"accepted_run": ACCEPTED_RUN, "phase1_binary_commit": PHASE1_COMMIT,
                  "dldi": DLDI, "hardware_status": "PENDING", "packages": {}}
    payloads = {}
    for variant, recorded in INPUTS.items():
        archives = list(args.inputs.rglob(f"TWiLightMenu-{variant}.tar.gz"))
        assert len(archives) == 1, (variant, "Expected exactly one accepted archive", archives)
        archive = archives[0]
        assert sha_file(archive) == recorded["archive_sha256"], (variant, "Archive SHA256 mismatch")
        payload, rows = {}, []
        with tarfile.open(archive, "r:gz") as tar:
            members = tar.getmembers()
            for source, identity in recorded["files"].items():
                matches = [m for m in members if m.name == "7zfile/" + source]
                assert len(matches) == 1, (source, "Missing or duplicate archive entry")
                member = matches[0]
                assert member.isfile() and member.size == identity["bytes"], (source, "Wrong type/size")
                with tar.extractfile(member) as f:
                    data = f.read(identity["bytes"] + 1)
                assert len(data) == identity["bytes"] and hashlib.sha256(data).hexdigest() == identity["sha256"], source
                target = identity["destination"]
                assert target == "BOOT.NDS" or (target.startswith("_nds/TWiLightMenu/") and target.endswith(".srldr"))
                assert target not in payload
                payload[target] = data
                rows.append([target, len(data), identity["sha256"], "replace launcher file", "7zfile/" + source])
        assert len(payload) == 9
        if variant == "on":
            payload[CONFIG_PATH] = CONFIG
            rows.append([CONFIG_PATH, len(CONFIG), hashlib.sha256(CONFIG).hexdigest(),
                         "second-pass config: create only after backing up/renaming any existing XL+ folder", "generated enabled test example"])
        out = args.output / (NAMES[variant] + ".zip")
        zip_bytes(out, payload)
        with (docs / (NAMES[variant] + "-MANIFEST.csv")).open("w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["SD-relative destination", "bytes", "SHA256", "copy action", "source entry"])
            w.writerows(sorted(rows))
        (docs / (NAMES[variant] + "-SHA256SUMS.txt")).write_text(
            "".join(hashlib.sha256(data).hexdigest() + "  " + path + "\n" for path, data in sorted(payload.items())))
        provenance["packages"][variant] = {
            "file_name": out.name, "zip_sha256": sha_file(out), "zip_bytes": out.stat().st_size,
            "source_commit": recorded["source_commit"], "source_archive_sha256": recorded["archive_sha256"],
            "launcher_files": 9, "sd_payload_files": len(payload), "config_included": variant == "on",
            "files": {r[0]: {"bytes": r[1], "sha256": r[2], "source": r[4]} for r in rows},
        }
        payloads[variant] = payload
    # Prove repackaging introduced no other binary differences.
    common = set(payloads["baseline"])
    assert common == set(payloads["off"])
    assert common | {CONFIG_PATH} == set(payloads["on"])
    off_changed = sorted(p for p in common if payloads["off"][p] != payloads["baseline"][p])
    on_changed = sorted(p for p in common if payloads["on"][p] != payloads["baseline"][p])
    assert off_changed == ["_nds/TWiLightMenu/settings.srldr"]
    assert on_changed == ["_nds/TWiLightMenu/dsimenu.srldr", "_nds/TWiLightMenu/main.srldr", "_nds/TWiLightMenu/settings.srldr"]
    provenance["changed_launcher_files_vs_baseline"] = {"off": off_changed, "on": on_changed}
    (docs / "PACKAGE-PROVENANCE.json").write_text(json.dumps(provenance, indent=2) + "\n")
    guide = Path(__file__).resolve().parents[1] / "docs" / "HARDWARE-TESTING.md"
    (docs / "START-HERE-Hardware-Test-Guide.md").write_bytes(guide.read_bytes())
    (docs / "ENABLED-CONFIG-EXAMPLE.txt").write_bytes(CONFIG)
    repository = Path(__file__).resolve().parents[2]
    (docs / "LICENSE-TWiLightMenu.txt").write_bytes((repository / "LICENSE").read_bytes())
    with (docs / "HARDWARE-RESULTS.csv").open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["test", "binary commit", "boot method", "TWiLight/nds-bootstrap versions before testing",
                    "cold boot", "restart", "settings", "buttons/touch", "manual", "original themes",
                    "disposable homebrew/DS game copy", "XL+ config status", "notes"])
        for variant, case in [("baseline", "baseline"), ("off", "foundation off"), ("on", "foundation on - config missing"), ("on", "foundation on - enabled config")]:
            w.writerow([case, INPUTS[variant]["source_commit"], "", ""] + ["NOT TESTED"] * 8 + [""])
    sums = []
    for p in sorted(args.output.glob("*.zip")):
        sums.append(sha_file(p) + "  " + p.name)
    (docs / "PACKAGE-SHA256SUMS.txt").write_text("\n".join(sums) + "\n")
    print(json.dumps(provenance, indent=2))


if __name__ == "__main__":
    main()
