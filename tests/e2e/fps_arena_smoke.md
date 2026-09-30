# FPS Arena Survival MVP smoke test

Run `./test_runner.sh --run` on a desktop host with raylib installed.

1. Confirm the game window opens with a crosshair and health/wave/enemy HUD.
2. Move with `W/A/S/D` and aim with the arrow keys; confirm the player remains within the enclosing walls.
3. Aim at a red standard enemy and press Space twice; confirm it disappears.
4. Clear waves until wave 3 and verify orange fast enemies appear. Continue to wave 5 and verify purple heavy enemies appear.
5. Allow enemies to reach the player; confirm health reaches zero and the game-over overlay freezes the action.
6. Press Enter; confirm health returns to 100 and a new wave 1 starts.

Optional evidence: capture an active-wave screenshot and a game-over screenshot.
