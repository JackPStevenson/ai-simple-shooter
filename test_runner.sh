#!/usr/bin/env bash

set -euo pipefail

readonly build_dir="build"
readonly compiler_flags=("-std=c++17" "-Wall" "-Wextra" "-Wpedantic" "-Isrc")

run_unit_tests() {
  mkdir -p "$build_dir"
  g++ "${compiler_flags[@]}" tests/game_rules_test.cpp src/game_rules.cpp \
    -o "$build_dir/game_rules_test"
  "$build_dir/game_rules_test"
}

build_game() {
  cmake -S . -B "$build_dir" -DCMAKE_BUILD_TYPE=Release
  cmake --build "$build_dir" --target fps_arena
}

case "${1:-}" in
  "")
    run_unit_tests
    build_game
    ;;
  --unit)
    run_unit_tests
    ;;
  --build)
    build_game
    ;;
  --run)
    build_game
    exec "$build_dir/fps_arena"
    ;;
  *)
    echo "Usage: $0 [--unit|--build|--run]" >&2
    exit 2
    ;;
esac
