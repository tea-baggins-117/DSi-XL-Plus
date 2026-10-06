#!/usr/bin/env bash
set -euo pipefail
xlplus_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
xlplus_test_dir=$(mktemp -d "${TMPDIR:-/tmp}/xlplus-tests.XXXXXX")
trap 'rm -rf -- "$xlplus_test_dir"' EXIT
flags=(-std=gnu++17 -O2 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -I"$xlplus_root/include")
if [[ ${SANITIZE:-0} == 1 ]]; then
    flags+=(-g -fsanitize=address,undefined -fno-omit-frame-pointer)
fi
core=("$xlplus_root"/src/core/*.cpp)
for test in config recovery storage_fault; do
    "${CXX:-g++}" "${flags[@]}" -DDSIXLPLUS_FOUNDATION=1 "${core[@]}" \
        "$xlplus_root/src/nds/xlplus_storage_nds.cpp" \
        "$xlplus_root/tests/xlplus_${test}_test.cpp" -o "$xlplus_test_dir/$test"
    "$xlplus_test_dir/$test"
done
"${CXX:-g++}" "${flags[@]}" -DDSIXLPLUS_FOUNDATION=0 "${core[@]}" \
    "$xlplus_root/tests/xlplus_recovery_test.cpp" -o "$xlplus_test_dir/recovery-off"
"$xlplus_test_dir/recovery-off"
