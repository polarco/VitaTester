#!/usr/bin/env bash
set -euo pipefail
ulimit -c 0
cd "$(dirname "$0")/.."
mkdir -p build-host
for mode in normal sanitized; do
  flags=(-std=c11 -Wall -Wextra -Werror -g -Isrc)
  if [[ "$mode" == sanitized ]]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie); fi
  cc "${flags[@]}" tests/test_core.c src/diagnostic.c src/safety.c src/queue.c -o "build-host/test-$mode"
  "build-host/test-$mode"
  cc "${flags[@]}" -D_POSIX_C_SOURCE=200809L -Itests/stubs tests/test_runtime.c src/diagnostic.c src/safety.c -o "build-host/runtime-$mode"
  "build-host/runtime-$mode"
done
python3 scripts/check-test-log.py
