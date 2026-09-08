#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
image='vitasdk/vitasdk@sha256:7f5eee50ff95b73c8c847dbfef6227aa0035886444b4d0254c21da8369ff1efc'
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD:/project" -w /project "$image" sh -c 'cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j2'
python3 scripts/verify-vpk.py build/VitaTester.vpk
