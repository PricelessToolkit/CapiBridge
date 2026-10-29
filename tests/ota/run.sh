#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd "$(dirname "$0")/../.." && pwd)"
test_binary="$(mktemp /tmp/capibridge-ota-test.XXXXXX)"
trap 'rm -f "$test_binary"' EXIT
c++ -std=c++17 -Wall -Wextra -Werror -Wno-deprecated-declarations \
  -I"$repo_root/lib/ArduinoJson/src" -I"$repo_root/tests/ota/stubs" -I"$repo_root/code/Gateway/ESP1" \
  "$repo_root/tests/ota/test_ota.cpp" \
  "$repo_root/code/Gateway/ESP1/firmware_update.cpp" \
  "$repo_root/code/Gateway/ESP1/settings.cpp" \
  "$repo_root/code/Gateway/ESP1/web_auth.cpp" -o "$test_binary"
"$test_binary"
