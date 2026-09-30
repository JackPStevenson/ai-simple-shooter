# Feature: Raylib FPS Arena Survival

## Feature Description

Create a small desktop first-person shooter MVP in C++ with raylib. The player survives in a compact 3D arena by moving with the keyboard, looking with the mouse, and firing an unlimited-ammo hitscan weapon. Enemies pursue the player in increasingly difficult endless waves. The game ends when player health reaches zero, then supports restarting the run.

The feature is a learning-oriented prototype, so it deliberately favors a clear game loop and deterministic game rules over content breadth or a general-purpose game-engine architecture.

## User Story

As a player learning from a small C++ game project,
I want to fight continuously escalating enemy waves in a first-person arena,
So that I can play a complete, understandable FPS loop and inspect its implementation.

## Problem Statement

The repository is an empty C++ executable. It contains no game loop, 3D renderer, input handling, game-state model, build integration for raylib, or tests for gameplay rules. A bounded, testable vertical slice is needed before expanding into a larger game.

## Solution Statement

Build a single-player Linux-desktop MVP around raylib's window, 3D camera, input, drawing, and collision primitives. Keep only wave formulas and archetype stats independent of raylib so they can be unit-tested without opening a window. Put the deliberately small runtime (player, enemy list, input, rendering, and restart state) in `main.cpp`; do not create an engine, ECS, scene graph, or renderer abstraction for this first version.

Use a capped roster of three enemy archetypes:

- Standard chaser: baseline speed, health, contact damage, and spawn weight; available from wave 1.
- Fast chaser: lower health and higher speed; becomes eligible in wave 3.
- Heavy chaser: higher health and lower speed; becomes eligible in wave 5.

An archetype is introduced every two waves, and no further archetypes are added after the third. Difficulty rises through a bounded spawn quota and a faster spawn interval, rather than indefinitely increasing health or speed. That keeps late waves playable and guarantees a fixed upper bound on simultaneous entities.

### Gameplay Constants and Progression Contract

These initial values are deliberately explicit so the implementation and tests share one definition of "simple". They are tuning constants, not an invitation to add a balancing system.

| Rule | Initial value |
| --- | --- |
| Player health | 100 |
| Weapon | 20 damage per hitscan shot; unlimited ammo; no cooldown required beyond one shot per mouse-button press |
| Standard enemy | 30 health, 2.0 units/second, 10 contact damage/second |
| Fast enemy | 20 health, 3.5 units/second, 8 contact damage/second |
| Heavy enemy | 80 health, 1.2 units/second, 16 contact damage/second |
| Wave quota | `min(2 + wave, 12)` enemies |
| Active-enemy cap | 8 live enemies |
| Spawn cadence | Starts at 1.0 second; decreases by 0.1 seconds per wave to a 0.5-second floor |
| Type eligibility | Standard: waves 1+; Fast: waves 3+; Heavy: waves 5+ |

Each wave must enqueue its complete quota, release enemies only while fewer than 8 are alive, and advance only after the queue and the live-enemy list are both empty. Waves therefore remain endless, but their difficulty plateaus at a bounded maximum rather than consuming more CPU/memory forever.

## Tech Stack

- C++17 (or the repository compiler's supported equivalent)
- raylib, installed with its CMake package available to `find_package(raylib CONFIG REQUIRED)`
- CMake 3.20 or later for the graphical target
- A shell build/test script retained as the project entry point; it must compile raylib-linked game and non-graphical unit-test binaries separately
- No additional engine, ECS, physics library, asset pipeline, or network dependency

## Relevant Files

### Existing Files

- `main.cpp` — currently the empty executable entry point; becomes only startup and shutdown wiring.
- `test_runner.sh` — currently compiles every root `*.cpp`; must become the explicit, safe build/test interface.
- `README.md` — documents the project's container workflow and must state host-display and raylib prerequisites accurately.

### New Files

- `src/game_rules.hpp` and `src/game_rules.cpp` — constants, archetype definitions, wave formulas, and pure combat/progression rules.
- `tests/game_rules_test.cpp` — deterministic tests for stats and wave progression, with no graphics dependency.
- `tests/e2e/fps_arena_smoke.md` — manual desktop test flow and screenshot evidence checklist.

## Commands

The implementation should make these commands valid from the repository root after raylib development files are installed:

```bash
./test_runner.sh
./test_runner.sh --unit
./test_runner.sh --build
./test_runner.sh --run
```

The no-argument command must run `--unit` followed by `--build`; it must never launch the interactive application. Build output belongs under the ignored `build/` directory. `--run` must first build, then execute `build/fps_arena`.

For the project's documented container workflow, use the container for the headless unit-test gate after its image has raylib development headers and the `raylib.pc` package metadata available:

```bash
docker run --rm -v "$(pwd)":/usr/src -w /usr/src -it cpp-container ./test_runner.sh --unit
```

`--unit` must never require raylib, an X/Wayland display, or a window. `--build` and `--run` require raylib. Run `--run` on the host desktop unless a separately documented display-forwarding setup is deliberately configured; the repository's current `docker run` examples do not provide one.

## Project Structure

```text
main.cpp                         -> small application entry point and game-loop wiring
src/game_rules.hpp/.cpp          -> pure health, wave, archetype, spawn, and hit-resolution rules
tests/game_rules_test.cpp        -> non-graphical tests for deterministic gameplay rules
tests/e2e/fps_arena_smoke.md     -> repeatable interactive end-to-end test procedure
test_runner.sh                   -> explicit build, unit-test, and interactive-run commands
README.md                        -> dependencies, controls, and local/container run instructions
specs/                           -> feature specifications such as this plan
```

## Code Style

- Use C++17, `#pragma once` headers, `PascalCase` for types/enums, and `camelCase` for functions and fields.
- Keep constants named and close to the game-rules module; do not scatter magic numbers through rendering code.
- Pass raylib values by value when they are lightweight value types; use `const` references for project-owned aggregate data.
- Keep stat and wave formulas free of `raylib.h`. `main.cpp` maps its small game state directly to raylib draw calls.

```cpp
struct WaveDefinition {
  int waveNumber;
  int spawnCount;
  int maxConcurrentEnemies;
  std::vector<EnemyType> eligibleTypes;
};

WaveDefinition buildWaveDefinition(int waveNumber) {
  WaveDefinition wave{waveNumber, std::min(2 + waveNumber, 12), 8,
                      {EnemyType::Standard}};
  if (waveNumber >= 3) wave.eligibleTypes.push_back(EnemyType::Fast);
  if (waveNumber >= 5) wave.eligibleTypes.push_back(EnemyType::Heavy);
  return wave;
}
```

## Implementation Plan

### Phase 1: Foundation

Define the dependency and build contract first. Add source directories and a raylib-aware `test_runner.sh` that separates headless logic tests from launching the visual game. Document the supported environment and controls, including the difference between a headless container test and a host-desktop graphical run. Put the gameplay constants above in one pure rules module.

### Phase 2: Core Implementation

Implement one deterministic game-rules module, then add the complete minimal runtime in `main.cpp`. Build player movement and a first-person raylib camera; implement click-to-fire hitscan targeting; add three chase-only enemy variants and contact damage; then implement endless waves, eligibility every two waves, a bounded spawn queue, death, and restart.

### Phase 3: Integration

Connect the minimal game state to primitive rendering only: arena floor/walls, colored enemy spheres, crosshair, health, current wave, enemies remaining, and a game-over/restart prompt. Validate through unit tests and a documented manual smoke test; screenshots are optional evidence, not a release requirement.

## Step by Step Tasks

### 1. Establish the raylib build and test contract

- Add the `src/` and `tests/e2e/` directories without moving unrelated repository files.
- Update `test_runner.sh` with `--unit`, `--build`, and `--run` modes, plus a safe no-argument `--unit` then `--build` default. Build into `build/` through CMake and raylib's exported package target, with `-std=c++17 -Wall -Wextra -Wpedantic` enabled for the game target.
- Make a missing CMake/raylib package prerequisite fail with an actionable message rather than a compiler error.
- Update `README.md` with the dependency prerequisite, the four commands defined above, controls, and the fact that `--run` needs a host display (the existing Docker command is only a headless unit-test example).
- Create `tests/e2e/fps_arena_smoke.md` now, before UI work, with the manual scenario below.

Acceptance:

- `--unit` has neither a graphical nor a raylib dependency; `--build` produces `build/fps_arena` using raylib's CMake package target.
- The README explains installation expectations, run commands, and controls.

Verify:

- `./test_runner.sh --unit`
- `./test_runner.sh --build`

Dependencies: None.

Likely files: `test_runner.sh`, `README.md`, `tests/e2e/fps_arena_smoke.md`, new source/test files.

### 2. Implement and test deterministic rules

- Define `EnemyType`, stat lookup, wave definition, archetype eligibility, spawn quota/cadence/concurrent cap, player damage, and hit damage in `src/game_rules.hpp/.cpp`.
- Make wave 1 standard-only, wave 3 admit fast enemies, and wave 5 admit heavy enemies; never add a fourth type.
- Use a deterministic input (seeded choice or supplied index) for enemy-type selection so tests do not depend on random distribution.
- Add `tests/game_rules_test.cpp` using the repository's smallest practical assertion-based test executable; do not add a test framework dependency.

Acceptance:

- Rules correctly model all three archetypes and their intended stat differences.
- Waves 1–2, 3–4, and 5+ expose exactly the intended eligible types.
- Spawn quota never exceeds 12, live enemies never exceed 8, and spawn cadence never drops below 0.5 seconds, including for a large wave number.

Verify:

- `./test_runner.sh --unit`

Dependencies: Task 1.

Likely files: `src/game_rules.hpp`, `src/game_rules.cpp`, `tests/game_rules_test.cpp`, `test_runner.sh`.

### 3. Add player, camera, arena, and hitscan combat

- Replace the placeholder `main.cpp` with the complete minimal game loop. Keep small `Player` and `Enemy` structs local to this file; do not introduce `FpsGame`, `GameWorld`, inheritance, or an ECS.
- Implement raylib window setup, a first-person `Camera3D`, `DisableCursor()`/`EnableCursor()` lifecycle, WASD movement, collision/clamping to arena bounds, Escape-to-quit, and a fixed crosshair.
- Represent the arena with a flat floor and simple enclosing walls; use no external assets.
- Add left-click hitscan: use `GetScreenToWorldRay()` at the viewport center, choose the nearest live enemy hit by `GetRayCollisionSphere()` within range, and apply fixed damage in the local `Enemy` list.

Acceptance:

- The player can move and look without leaving the arena.
- A visible target under the crosshair takes damage from a left-click; shots have unlimited ammunition and no reload behavior.

Verify:

- `./test_runner.sh --build`
- Follow movement/look/shoot steps in `tests/e2e/fps_arena_smoke.md`.

Dependencies: Tasks 1–2.

Likely files: `main.cpp`.

### 4. Add enemy behavior, damage, and endless-wave progression

- Add enemy spawning at safe perimeter positions outside a minimum player radius.
- Update each living enemy each frame with direct chase steering, contact-range damage, and removal on death. Clamp simulation delta time before updating movement so a long frame cannot tunnel entities through the arena. Do not implement obstacle avoidance or enemy-to-enemy separation in the MVP.
- Start wave 1 automatically. When its spawn queue is exhausted and all enemies are dead, begin the next wave after a short visible inter-wave delay.
- Introduce fast enemies at wave 3 and heavies at wave 5. Use only the three capped types thereafter; obey the documented 12-enemy quota, 8-live-enemy cap, and 0.5-second spawn-cadence floor.
- On zero player health, freeze simulation and show a game-over overlay; pressing Enter fully resets health, enemies, spawn queue, wave number, and timers.

Acceptance:

- Enemies chase and damage the player on contact.
- Enemy composition changes at waves 3 and 5 exactly as specified.
- Surviving clears wave after wave without unbounded simultaneous entities; dying and pressing Enter starts a clean new run.

Verify:

- `./test_runner.sh --unit`
- `./test_runner.sh --build`
- Complete the wave/death/restart scenario in `tests/e2e/fps_arena_smoke.md`.

Dependencies: Tasks 2–3.

Likely files: `main.cpp`, `tests/game_rules_test.cpp`.

### 5. Render HUD, finalize E2E evidence, and run regression checks

- Render distinct colors for standard, fast, and heavy enemy spheres.
- Render only current health, wave number, and enemies remaining in a readable HUD.
- Complete the E2E smoke procedure with exact player actions and expected observations: launch, move/look, defeat standard enemy, observe fast at wave 3, observe heavy at wave 5, die, and restart. Capture screenshots only when the test environment makes that convenient.
- Keep visual effects intentionally restrained: no audio, external textures, particles, pickups, menus, or save data.

Acceptance:

- Players can understand health, wave progression, and enemy distinctions without reading source code.
- The documented smoke test covers the required end-to-end loop; screenshots are optional evidence.

Verify:

- `./test_runner.sh --unit`
- `./test_runner.sh --build`
- `./test_runner.sh --run`
- Execute and complete `tests/e2e/fps_arena_smoke.md`.

Dependencies: Task 4.

Likely files: `main.cpp`, `tests/e2e/fps_arena_smoke.md`, `README.md`.

### 6. Run the validation commands

- Run every command in the Validation Commands section from a clean build output state.
- Record any host/container raylib prerequisite discrepancy in the README; do not silently weaken the test or remove a check.

Acceptance:

- All automated checks exit successfully.
- The interactive smoke test passes on a display-capable desktop environment.

Verify:

- Execute the full command list below.

Dependencies: Tasks 1–5.

Likely files: no intended source changes unless a documented validation failure reveals one.

## Testing Strategy

### Unit Tests

- Archetype stats exactly match the gameplay-constants table: fast is quicker and less durable than standard; heavy is slower and more durable.
- Eligibility boundaries: waves 1–2 standard-only; 3–4 standard/fast; 5 and later standard/fast/heavy.
- Progression bounds: quota never exceeds 12, concurrent count never exceeds 8, and spawn cadence never drops below 0.5 seconds, including an intentionally large wave number.
- Combat rules: an in-range hit reduces health and removes an enemy exactly at zero; non-positive player health produces the game-over state.

### Interactive Smoke Test

- Manual smoke coverage: a queue never releases an enemy while 8 are live; wave advancement waits for both an empty queue and no live enemies; contact damage never displays health below zero.
- Manual restart coverage: a new run restores 100 player health and wave 1, with no enemies visible from the prior run.

### Edge Cases

- A player holds movement against each wall or corner and remains inside the arena.
- A shot ray overlaps more than one enemy; only the nearest valid target is damaged.
- No new wave begins until both queued spawns and live enemies are exhausted.
- A large wave cannot create more than the active-enemy cap at once.
- The player dies during a spawn interval; simulation stops and no further damage/spawns occur until restart.
- Restarting repeatedly does not accumulate enemy entities, timers, or health from prior runs.
- Frame-time spikes cannot move enemies/player through arena boundaries or generate negative health/counts.

## Acceptance Criteria

- The application compiles as a C++ raylib desktop executable using the documented commands.
- The player can WASD-move, mouse-look, and fire an unlimited-ammo hitscan weapon in a simple 3D enclosed arena.
- Standard, fast, and heavy chase enemies are visually distinct and use their respective speed/health profiles.
- Wave 1 begins with standard enemies, wave 3 introduces fast enemies, and wave 5 introduces heavy enemies; no additional archetypes are created.
- Waves continue indefinitely, with a maximum quota of 12, a maximum of 8 concurrent enemies, and a spawn-interval floor of 0.5 seconds.
- Enemy contact depletes player health; death freezes the game and Enter begins a clean new run.
- HUD exposes at least player health and the active wave.
- Non-graphical gameplay tests pass, and the documented interactive E2E smoke flow succeeds.
- The implementation adds none of: multiplayer, pickups, reload/ammunition mechanics, extra weapons, saves, menus, audio, external art assets, or advanced pathfinding.

## Validation Commands

Execute every command from the repository root after installing raylib development files:

```bash
./test_runner.sh --unit
./test_runner.sh --build
./test_runner.sh --run
```

Then complete every action in:

```bash
tests/e2e/fps_arena_smoke.md
```

Interactive validation runs only in a desktop session with an available display; the headless automated gate is `./test_runner.sh --unit`. Screenshots of an active wave and game-over overlay are useful but optional test evidence.

## Risks and Mitigations

| Risk | Impact | Mitigation |
| --- | --- | --- |
| raylib is absent from the base container | Graphical build cannot start | Let CMake report the missing raylib package and document that unit tests remain runnable without raylib. |
| Container has no forwarded display | Interactive game cannot open | Keep the container command headless-only; run `--run` on the host desktop unless display forwarding is intentionally added and documented. |
| Gameplay rules become tied to rendering | Tests require a display and become brittle | Keep archetype stats and wave formulas in the pure `game_rules` module. |
| Endless mode grows process work without bound | Late waves become unplayable or unstable | Cap active enemies and clamp all count/stat scaling. |
| Chase enemies overlap or trap the player unfairly | Core loop feels rough | Use minimum spawn distance, arena clamping, and bounded contact-damage intervals; defer separation/pathfinding until after MVP feedback. |
| First-person controls feel disorienting | Manual validation is inconclusive | Use standard WASD/mouse controls, an explicit crosshair, bounded movement, and simple geometry. |

## Notes

- Primary target is a Linux desktop C++ workflow with headless container unit tests. raylib remains portable, but platform-specific installers and Docker display forwarding are outside this implementation scope.
- Default controls: `W/A/S/D` move, arrow keys aim, Space fires, Enter restarts after death, and Escape exits.
- MVP boundary: no source layout beyond `main.cpp` plus `game_rules`, no renderer/world abstraction, no enemy separation, no settings, no pause screen, no score system, and no tuning UI.
- This plan intentionally does not modify the existing unrelated whitespace-only worktree changes.

## Sources

- raylib's current cheatsheet documents `DisableCursor()` and `GetScreenToWorldRay()`: https://www.raylib.com/cheatsheet/raylib_cheatsheet_v6.0.pdf
- raylib's collision API documents `GetRayCollisionSphere()`: https://www.raylib.com/cheatsheet/raylib_cheatsheet_v5.5.pdf
- raylib's official source exposes desktop platform and build configuration options: https://github.com/raysan5/raylib/blob/master/CMakeOptions.txt
