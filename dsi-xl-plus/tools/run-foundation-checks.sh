#!/usr/bin/env bash
# Build evidence only. Never installs to an SD card or writes Phase 0 archives.
set -euo pipefail
BASE=97b22fe2cc282cee5bae7c12e0db54c1aa111dfb
DLDI=4e3fba1f4a96dc0ee53b97cecbfe497633b7dfd5
BASE_IMAGE=devkitpro/devkitarm:20241104
BASE_DIGEST=sha256:a998edf6b06416b5c053edbcd879abfa22b1b88e9cd3f267f5c4ee9fec71a93a
MANUAL_SHA=72b75b98600ce78c3995803f1aa87b9a94384778d7bd6b5d20520ff62f826614
variant=${1:-all}
case "$variant" in baseline|off|on|all) ;; *) echo 'Usage: bash run-foundation-checks.sh [baseline|off|on|all]' >&2; exit 2;; esac
repo=$(git -C "$(dirname -- "${BASH_SOURCE[0]}")" rev-parse --show-toplevel)
for command in git docker curl 7z python3 sha256sum tar; do
    command -v "$command" >/dev/null || { echo "Missing required command: $command" >&2; exit 127; }
done
[[ $(git -C "$repo" branch --show-current) != master ]] || { echo 'Refusing a development run from master.' >&2; exit 2; }
[[ -z $(git -C "$repo" status --porcelain) ]] || { echo 'Commit or preserve working changes before running clean build checks.' >&2; exit 2; }
head=$(git -C "$repo" rev-parse HEAD)
git -C "$repo" merge-base --is-ancestor "$BASE" "$head"
git -C "$repo" describe --tags --abbrev=0 "$BASE" >/dev/null || { echo "Fetch reachable tags/history for baseline version reporting." >&2; exit 2; }
docker info >/dev/null
parent=${XLPLUS_OUTPUT_PARENT:-"$HOME/Projects/DSiXLPlus-Phase1-Builds"}
parent=$(realpath -m -- "$parent")
case "$parent/" in */DSiXLPlus-Baselines/*|*/DSi-XL-Plus-Phase0/*) echo 'Phase 0 output locations are protected.' >&2; exit 2;; esac
mkdir -p -- "$parent"
run=$(mktemp -d "$parent/$(date -u +%Y%m%dT%H%M%SZ)-${head:0:7}-$variant-XXXXXX")
mkdir -p "$run/evidence" "$run/image-context"
stage=preflight
finish() {
    rc=$?
    trap - EXIT
    printf 'exit_code=%s\nlast_stage=%s\ncommit=%s\nhardware=PENDING\n' "$rc" "$stage" "$head" > "$run/evidence/RESULT.txt"
    echo "Evidence: $run"
    exit "$rc"
}
trap finish EXIT
logged() { local log=$1; shift; "$@" 2>&1 | tee "$run/evidence/$log"; }
git -C "$repo" diff --name-status "$BASE" "$head" > "$run/evidence/changed-files.txt"
git -C "$repo" log -1 --format=fuller > "$run/evidence/commit.txt"
cp "$repo/Dockerfile" "$run/evidence/Dockerfile.upstream"
stage=docker-image
logged docker-pull.log docker pull "$BASE_IMAGE"
docker image inspect "$BASE_IMAGE" > "$run/evidence/base-image.json"
python3 - "$run/evidence/base-image.json" "$BASE_DIGEST" <<'PY'
import json, sys
image = json.load(open(sys.argv[1]))[0]
assert any(d.endswith('@' + sys.argv[2]) for d in image['RepoDigests']), 'Pinned tag digest changed; diagnose environment first'
PY
image_tag="dsi-xl-plus-foundation:$(basename "$run" | tr '[:upper:]' '[:lower:]')"
if ! logged docker-build.log docker build --tag "$image_tag" --label twilightmenu \
    --file "$repo/Dockerfile" "$run/image-context"; then
    # Only the same known apt workaround already present in upstream CI.
    if grep -q 'bullseye-security' "$run/evidence/docker-build.log" && \
        grep -Eq 'does not have a Release file|404[[:space:]]+Not Found' "$run/evidence/docker-build.log"; then
        python3 - "$repo/Dockerfile" "$run/evidence/Dockerfile.ci-apt" <<'PY'
import pathlib, sys
text = pathlib.Path(sys.argv[1]).read_text()
assert text.startswith('FROM devkitpro/devkitarm:20241104\n')
lines = text.splitlines(keepends=True)
lines.insert(1, 'RUN sed -i "/bullseye-security/d" /etc/apt/sources.list\n')
pathlib.Path(sys.argv[2]).write_text(''.join(lines))
PY
        logged docker-build-ci-apt.log docker build --tag "$image_tag" --label twilightmenu \
            --file "$run/evidence/Dockerfile.ci-apt" "$run/image-context"
    else exit 1; fi
fi
docker image inspect "$image_tag" > "$run/evidence/build-image.json"
image_id=$(docker image inspect --format '{{.Id}}' "$image_tag")
logged toolchain.log docker run --rm "$image_id" bash -c \
    'set -e; "${DEVKITARM:?}/bin/arm-none-eabi-gcc" --version; make --version; command -v grit mmutil ndstool; if command -v dkp-pacman >/dev/null; then dkp-pacman -Q; else pacman -Q; fi'
stage=manual-input
if [[ -n ${XLPLUS_MANUAL_ARCHIVE:-} ]]; then
    cp -- "$XLPLUS_MANUAL_ARCHIVE" "$run/manual-pages.7z"
else
    logged manual-download.log curl --fail --location --retry 2 --output "$run/manual-pages.7z" \
        https://github.com/DS-Homebrew/twilight-manual/releases/download/pages/pages.7z
fi
printf '%s  %s\n' "$MANUAL_SHA" "$run/manual-pages.7z" | sha256sum -c -
sha256sum "$run/manual-pages.7z" > "$run/evidence/manual-pages.sha256"

build_variant() {
    local name=$1 revision=$head flag=0
    [[ $name != baseline ]] || revision=$BASE
    [[ $name != on ]] || flag=1
    local directory="$run/$name" source="$run/$name/source"
    mkdir -p "$directory/evidence"
    stage="$name-clone"
    git clone --no-hardlinks --no-checkout "$repo" "$source" > "$directory/evidence/clone.log" 2>&1
    git -C "$source" checkout --detach "$revision" >> "$directory/evidence/clone.log" 2>&1
    git -C "$source" remote set-url origin https://github.com/tea-baggins-117/DSi-XL-Plus.git
    git -C "$source" submodule update --init --recursive > "$directory/evidence/submodule.log" 2>&1
    [[ $(git -C "$source/booter_fc/flashcart_specifics/DLDI" rev-parse HEAD) == "$DLDI" ]]
    [[ -z $(git -C "$source" status --porcelain) ]]
    git -C "$source" rev-parse HEAD > "$directory/evidence/source-commit.txt"
    git -C "$source" submodule status --recursive > "$directory/evidence/submodules.txt"
    7z x -y "-o$source/manual/nitrofiles" "$run/manual-pages.7z" > "$directory/evidence/manual-extract.log"
    local docker_args=(run --rm --user "$(id -u):$(id -g)" --mount "type=bind,source=$source,target=/data" --workdir /data)
    stage="$name-make-package"
    set +e
    docker "${docker_args[@]}" --env MAKEFLAGS=-j1 "$image_id" make package "XLPLUS=$flag" 2>&1 | tee "$directory/evidence/build.log"
    local statuses=("${PIPESTATUS[@]}")
    set -e
    printf '%s\n' "${statuses[0]}" > "$directory/evidence/build.exit-code"
    [[ ${statuses[0]} == 0 && ${statuses[1]} == 0 ]] || return 1
    stage="$name-verify"
    git -C "$source" diff --exit-code HEAD -- . ':(exclude)7zfile' ':(exclude)manual/nitrofiles' > "$directory/evidence/post-build-source.diff"
    git -C "$source" status --short > "$directory/evidence/post-build-status.txt"
    [[ $(git -C "$source/booter_fc/flashcart_specifics/DLDI" rev-parse HEAD) == "$DLDI" ]]
    for output in 'DSi&3DS - SD card users/BOOT.NDS' 'Flashcard users/BOOT.NDS' \
        '_nds/TWiLightMenu/main.srldr' '_nds/TWiLightMenu/dsimenu.srldr' '_nds/TWiLightMenu/settings.srldr' \
        '_nds/TWiLightMenu/akmenu.srldr' '_nds/TWiLightMenu/r4menu.srldr' '_nds/TWiLightMenu/mainmenu.srldr' \
        '_nds/TWiLightMenu/manual.srldr' '_nds/TWiLightMenu/slot1launch.srldr' '_nds/TWiLightMenu/3dssplash.srldr' \
        '_nds/TWiLightMenu/gbapatcher.srldr' 'Multimedia/_nds/TWiLightMenu/imageview.srldr'; do
        [[ -s "$source/7zfile/$output" ]] || { echo "Missing output: $output" >&2; return 1; }
    done
    docker "${docker_args[@]}" "$image_id" bash -c '
        set -e
        for component in title romsel_dsimenutheme settings; do
            "${DEVKITARM}/bin/arm-none-eabi-size" "$component/arm9/$component.elf"
        done
    ' > "$directory/evidence/elf-sizes.txt"
    for component in title romsel_dsimenutheme settings; do
        docker "${docker_args[@]}" "$image_id" bash -c 'exec "${DEVKITARM}/bin/arm-none-eabi-nm" -C -S --size-sort "$1/arm9/$1.elf"' bash "$component" > "$directory/evidence/$component-symbols.txt"
        if [[ $flag == 1 ]]; then
            grep -q 'dsi_xl_plus::' "$directory/evidence/$component-symbols.txt"
        else
            if grep -q 'dsi_xl_plus::' "$directory/evidence/$component-symbols.txt"; then echo 'Compile-off symbols found' >&2; return 1; fi
        fi
    done
    python3 - "$source" "$directory/evidence" <<'PY'
from pathlib import Path
import hashlib, json, shutil, sys
src, out = map(Path, sys.argv[1:])
files = {}
for p in sorted((src/'7zfile').rglob('*')):
    if p.is_file(): files[str(p.relative_to(src/'7zfile'))] = {'bytes': p.stat().st_size, 'sha256': hashlib.file_digest(p.open('rb'), 'sha256').hexdigest()}
(out/'package-files.json').write_text(json.dumps(files, indent=2)+'\n')
for p in src.rglob('*.map'):
    target = out/'maps'/p.relative_to(src)
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(p, target)
PY
    tar -czf "$directory/TWiLightMenu-$name.tar.gz" -C "$source" 7zfile
    sha256sum "$directory/TWiLightMenu-$name.tar.gz" > "$directory/evidence/package-archive.sha256"
    printf 'BUILD_PASSED_HARDWARE_UNTESTED\n' > "$directory/evidence/RESULT.txt"
}
if [[ $variant == all ]]; then
    for item in baseline off on; do build_variant "$item"; done
else
    build_variant "$variant"
fi
stage=complete
