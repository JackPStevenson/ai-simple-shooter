#include "game_rules.hpp"

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {
constexpr int kWidth = 1280;
constexpr int kHeight = 720;
constexpr int kStartingHealth = 100;
constexpr int kShotDamage = 20;
constexpr float kArenaHalfSize = 20.0F;
constexpr float kPlayerHeight = 1.8F;
constexpr float kEnemyRadius = 0.65F;
constexpr float kContactRange = 1.3F;
constexpr float kShotRange = 30.0F;
constexpr float kPlayerMoveSpeed = 8.0F;
constexpr float kAimSpeed = 90.0F;

struct Enemy {
  EnemyType type;
  Vector3 position;
  int health;
};

Color colorFor(EnemyType type) {
  if (type == EnemyType::Fast) return ORANGE;
  if (type == EnemyType::Heavy) return PURPLE;
  return RED;
}

float distanceOnFloor(Vector3 left, Vector3 right) {
  const float x = left.x - right.x;
  const float z = left.z - right.z;
  return std::sqrt(x * x + z * z);
}

Vector3 spawnPosition(int index) {
  const float offset = -12.0F + static_cast<float>((index * 7) % 25);
  switch (index % 4) {
    case 0: return {offset, kEnemyRadius, -18.5F};
    case 1: return {18.5F, kEnemyRadius, offset};
    case 2: return {offset, kEnemyRadius, 18.5F};
    default: return {-18.5F, kEnemyRadius, offset};
  }
}

void drawArena() {
  DrawPlane({0.0F, 0.0F, 0.0F}, {40.0F, 40.0F}, DARKGRAY);
  DrawGrid(20, 2.0F);
  DrawCube({0.0F, 1.0F, -20.0F}, 40.0F, 2.0F, 0.5F, GRAY);
  DrawCube({0.0F, 1.0F, 20.0F}, 40.0F, 2.0F, 0.5F, GRAY);
  DrawCube({-20.0F, 1.0F, 0.0F}, 0.5F, 2.0F, 40.0F, GRAY);
  DrawCube({20.0F, 1.0F, 0.0F}, 0.5F, 2.0F, 40.0F, GRAY);
}

void resetRun(Camera3D* camera, float* health, int* waveNumber,
              WaveDefinition* wave, int* queued, int* spawnIndex,
              float* spawnTimer, std::vector<Enemy>* enemies) {
  camera->position = {0.0F, kPlayerHeight, 0.0F};
  camera->target = {0.0F, kPlayerHeight, 1.0F};
  *health = kStartingHealth;
  *waveNumber = 1;
  *wave = buildWaveDefinition(*waveNumber);
  *queued = wave->spawnQuota;
  *spawnIndex = 0;
  *spawnTimer = 0.0F;
  enemies->clear();
}
}  // namespace

int main() {
  InitWindow(kWidth, kHeight, "FPS Arena Survival MVP");
  if (!IsWindowReady()) {
    return 1;
  }
  SetTargetFPS(60);

  Camera3D camera{{0.0F, kPlayerHeight, 0.0F}, {0.0F, kPlayerHeight, 1.0F},
                  {0.0F, 1.0F, 0.0F}, 60.0F, CAMERA_PERSPECTIVE};
  float health = static_cast<float>(kStartingHealth);
  int waveNumber = 1;
  WaveDefinition wave = buildWaveDefinition(waveNumber);
  int queued = wave.spawnQuota;
  int spawnIndex = 0;
  float spawnTimer = 0.0F;
  std::vector<Enemy> enemies;

  while (!WindowShouldClose()) {
    const bool gameOver = health <= 0;
    if (gameOver && IsKeyPressed(KEY_ENTER)) {
      resetRun(&camera, &health, &waveNumber, &wave, &queued, &spawnIndex,
               &spawnTimer, &enemies);
    }
    if (!gameOver) {
      const float frameTime = std::min(GetFrameTime(), 0.05F);
      Vector3 movement{};
      if (IsKeyDown(KEY_W)) movement.x += kPlayerMoveSpeed * frameTime;
      if (IsKeyDown(KEY_S)) movement.x -= kPlayerMoveSpeed * frameTime;
      if (IsKeyDown(KEY_D)) movement.y += kPlayerMoveSpeed * frameTime;
      if (IsKeyDown(KEY_A)) movement.y -= kPlayerMoveSpeed * frameTime;
      Vector3 rotation{};
      if (IsKeyDown(KEY_RIGHT)) rotation.x += kAimSpeed * frameTime;
      if (IsKeyDown(KEY_LEFT)) rotation.x -= kAimSpeed * frameTime;
      if (IsKeyDown(KEY_DOWN)) rotation.y += kAimSpeed * frameTime;
      if (IsKeyDown(KEY_UP)) rotation.y -= kAimSpeed * frameTime;
      UpdateCameraPro(&camera, movement,
                      rotation,
                      0.0F);
      const Vector3 offset = {camera.target.x - camera.position.x,
                              camera.target.y - camera.position.y,
                              camera.target.z - camera.position.z};
      camera.position.x = std::clamp(camera.position.x, -19.0F, 19.0F);
      camera.position.z = std::clamp(camera.position.z, -19.0F, 19.0F);
      camera.position.y = kPlayerHeight;
      camera.target = {camera.position.x + offset.x, camera.position.y + offset.y,
                       camera.position.z + offset.z};

      spawnTimer += frameTime;
      if (queued > 0 && static_cast<int>(enemies.size()) < wave.maxConcurrentEnemies &&
          spawnTimer >= wave.spawnIntervalSeconds) {
        const EnemyType type = wave.eligibleTypes[static_cast<size_t>(spawnIndex) %
                                                   wave.eligibleTypes.size()];
        enemies.push_back({type, spawnPosition(spawnIndex), enemyStats(type).health});
        ++spawnIndex;
        --queued;
        spawnTimer = 0.0F;
      }

      for (Enemy& enemy : enemies) {
        const EnemyStats stats = enemyStats(enemy.type);
        const float distance = distanceOnFloor(enemy.position, camera.position);
        if (distance > kContactRange) {
          enemy.position.x += (camera.position.x - enemy.position.x) / distance * stats.speed * frameTime;
          enemy.position.z += (camera.position.z - enemy.position.z) / distance * stats.speed * frameTime;
        } else {
          health = std::max(0.0F, health - stats.contactDamagePerSecond * frameTime);
        }
      }

      if (IsKeyPressed(KEY_SPACE)) {
        const Ray ray = GetScreenToWorldRay({kWidth / 2.0F, kHeight / 2.0F}, camera);
        int target = -1;
        float nearest = kShotRange;
        for (size_t index = 0; index < enemies.size(); ++index) {
          const RayCollision hit = GetRayCollisionSphere(ray, enemies[index].position, kEnemyRadius);
          if (hit.hit && hit.distance < nearest) {
            target = static_cast<int>(index);
            nearest = hit.distance;
          }
        }
        if (target >= 0) enemies[static_cast<size_t>(target)].health -= kShotDamage;
      }
      enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                                   [](const Enemy& enemy) { return enemy.health <= 0; }),
                    enemies.end());
      if (queued == 0 && enemies.empty()) {
        wave = buildWaveDefinition(++waveNumber);
        queued = wave.spawnQuota;
        spawnTimer = 0.0F;
      }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(camera);
    drawArena();
    for (const Enemy& enemy : enemies) DrawSphere(enemy.position, kEnemyRadius, colorFor(enemy.type));
    EndMode3D();
    DrawText(TextFormat("Health: %d", static_cast<int>(std::ceil(health))), 20, 20, 24, MAROON);
    DrawText(TextFormat("Wave: %d", waveNumber), 20, 50, 24, DARKBLUE);
    DrawText(TextFormat("Enemies remaining: %d", queued + static_cast<int>(enemies.size())), 20, 80, 24, DARKGREEN);
    DrawText("+", kWidth / 2 - 6, kHeight / 2 - 12, 24, BLACK);
    if (gameOver) {
      DrawRectangle(0, 0, kWidth, kHeight, Fade(BLACK, 0.6F));
      DrawText("GAME OVER", kWidth / 2 - 100, kHeight / 2 - 30, 40, RAYWHITE);
      DrawText("Press Enter to restart", kWidth / 2 - 125, kHeight / 2 + 25, 24, RAYWHITE);
    }
    EndDrawing();
  }
  CloseWindow();
}
