# Spec: Raylib FPS Arena Survival MVP

## Objective

Build a compact desktop first-person shooter in C++ for a learning project. A player moves in a simple 3D arena, aims with arrow keys, fires an unlimited-ammo hitscan weapon, and survives endless enemy waves. The run ends at zero health and can be restarted immediately.

The intended user is a developer/player who wants one complete FPS loop that is small enough to understand and extend. Success is a locally runnable game with three clear enemy types, bounded endless progression, a minimal HUD, and a repeatable verification path.

### User Story

As a player learning from a small C++ game project,
I want to fight escalating enemy waves in a first-person arena,
So that I can play and inspect a complete FPS survival loop.

## Tech Stack

- C++17
- raylib 5.5 or later, installed as a system dependency with its CMake package available to `find_package(raylib CONFIG REQUIRED)`
- CMake 3.20 or later for the graphical target; the shell runner remains the developer-facing command interface
- Standard library only for game state; no ECS, physics engine, asset library, test framework, or network dependency
- Bash `test_runner.sh` as the build and validation interface

## Commands

After installing raylib development headers, libraries, and CMake, all commands run from the repository root:

```bash
./test_runner.sh          # unit tests, then graphical build; never launches a window
./test_runner.sh --unit   # headless rules tests only
./test_runner.sh --build  # builds build/fps_arena
./test_runner.sh --run    # builds, then launches build/fps_arena on a desktop host
```

The current Docker command is suitable only for headless unit validation:

```bash
docker run --rm -v "$(pwd)":/usr/src -w /usr/src -it cpp-container ./test_runner.sh --unit
```

`--run` requires a host desktop display unless Docker display forwarding is separately configured; display forwarding is not part of this MVP.

## Project Structure

```text
main.cpp                         -> entire minimal raylib game loop and local Player/Enemy structs
 CMakeLists.txt                  -> imports raylib's exported CMake target for complete static-library link dependencies
src/game_rules.hpp/.cpp          -> enemy stats, wave formula, caps, and eligibility rules
tests/game_rules_test.cpp        -> headless assertion-based game-rules tests
tests/e2e/fps_arena_smoke.md     -> manual desktop smoke-test procedure
test_runner.sh                   -> build/test/run modes and compiler diagnostics
README.md                        -> prerequisites, commands, and controls
specs/                           -> approved plan and this implementation specification
```

## Functional Requirements

### Controls and arena

- `W`, `A`, `S`, and `D` move the player; arrow keys control first-person aim.
- Space fires one 20-damage hitscan shot with unlimited ammunition and no reload mechanic.
- Escape exits. Enter starts a clean run after death.
- The arena is an enclosed floor-and-walls scene made only from raylib primitives; no external assets are required.
- Player movement is clamped inside the arena. A fixed crosshair appears at screen center.

### Enemy types

| Type | Availability | Health | Speed | Contact damage |
| --- | --- | ---: | ---: | ---: |
| Standard | Wave 1+ | 30 | 2.0 units/sec | 10/sec |
| Fast | Wave 3+ | 20 | 3.5 units/sec | 8/sec |
| Heavy | Wave 5+ | 80 | 1.2 units/sec | 16/sec |

- Enemies use direct chase movement only.
- Enemies are visually distinct through color; no models, textures, particles, special attacks, obstacle avoidance, or enemy-to-enemy separation are in scope.
- A centered shot selects and damages the closest ray-intersected live enemy in range.

### Wave progression and game state

- Player starting health is 100.
- Wave quota is `min(2 + waveNumber, 12)`.
- A wave spawns enemies every 1.0 second, decreasing by 0.1 seconds each wave to a 0.5-second floor.
- At most eight enemies may be alive simultaneously. Remaining quota stays queued until a live enemy dies.
- Waves 1–2 contain only standard enemies. Waves 3–4 may contain standard and fast enemies. Wave 5 onward may contain all three types.
- A new wave begins only after the current wave has no queued or live enemies.
- On player death, simulation stops and a game-over/restart prompt appears. Enter resets health, wave number, enemies, queue, and timers.
- The game continues indefinitely, while wave quota, spawn rate, and live enemy count remain bounded.

### HUD

- Show player health, current wave, and enemies remaining.
- Show a game-over prompt with restart instruction after death.

## Code Style

- Use `PascalCase` for types and enums; use `camelCase` for functions and fields.
- Compile with `-std=c++17 -Wall -Wextra -Wpedantic`.
- Keep only stats and progression formulas in `game_rules`; keep the MVP runtime in `main.cpp`.
- Link the graphical target through raylib's exported CMake target so static-library dependencies are carried by the package instead of duplicated in shell flags.
- Keep named constants close to `game_rules`; do not duplicate balancing values in rendering code.

```cpp
WaveDefinition buildWaveDefinition(int waveNumber) {
  WaveDefinition wave{waveNumber, std::min(2 + waveNumber, 12), 8,
                      {EnemyType::Standard}};
  if (waveNumber >= 3) wave.eligibleTypes.push_back(EnemyType::Fast);
  if (waveNumber >= 5) wave.eligibleTypes.push_back(EnemyType::Heavy);
  return wave;
}
```

## Testing Strategy

### Headless unit tests

`tests/game_rules_test.cpp` must test:

- exact stats for each archetype;
- type eligibility at waves 1, 3, and 5;
- quota cap of 12, live-enemy cap of 8, and spawn-cadence floor of 0.5 seconds at large wave numbers;
- wave formula values at low-number boundaries.

The test executable must have no raylib or display dependency.

### Interactive smoke test

`tests/e2e/fps_arena_smoke.md` must describe these observable steps:

1. Launch the game and verify movement, arrow-key aim, and crosshair.
2. Shoot a standard enemy and verify it dies after expected hits.
3. Reach wave 3 and verify a fast enemy appears; reach wave 5 and verify a heavy enemy appears.
4. Let enemies deal lethal contact damage; verify the game-over prompt and frozen simulation.
5. Press Enter and verify health returns to 100 and a fresh wave 1 begins.

Screenshots of an active wave and game-over screen are useful evidence when convenient, but are not required for MVP completion.

## Boundaries

- Always: preserve the unit-test/graphical-run separation; run `./test_runner.sh --unit` and `./test_runner.sh --build` before handoff; keep all entity counts bounded; document prerequisite failures clearly.
- Ask first: adding or changing third-party dependencies, changing the Docker image or CI, moving beyond C++17, changing the stated balance constants, or adding persistent data.
- Never: add multiplayer, ammo/reloads, pickups, additional weapons, score/progression systems, menus/settings/pause screens, saves, audio, external art assets, advanced AI/pathfinding, an ECS, or a renderer/world abstraction.

## Success Criteria

- The four documented commands behave exactly as specified.
- Unit tests pass without raylib or a graphical display.
- The raylib executable builds with the documented compiler warnings enabled.
- A player can move, look, shoot, take contact damage, die, and restart.
- Enemy availability is standard-only through wave 2, adds fast at wave 3, and adds heavy at wave 5.
- Endless progression obeys the quota, concurrent-enemy, and cadence caps.
- Health, wave, and enemies remaining are visible during play.
- The manual smoke test passes on a display-capable desktop host.

## Open Questions

None for the MVP. Post-MVP tuning (arena size, hit range, and the listed numeric balance values) must be based on play feedback and requires a spec update before implementation.

## Sources

- raylib `DisableCursor()` and `GetScreenToWorldRay()`: https://www.raylib.com/cheatsheet/raylib_cheatsheet_v6.0.pdf
- raylib `GetRayCollisionSphere()`: https://www.raylib.com/cheatsheet/raylib_cheatsheet_v5.5.pdf
