#include "game_rules.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

void expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << '\n';
    std::exit(1);
  }
}

bool nearlyEqual(float left, float right) {
  return std::fabs(left - right) < 0.001F;
}

}  // namespace

int main() {
  const EnemyStats standard = enemyStats(EnemyType::Standard);
  const EnemyStats fast = enemyStats(EnemyType::Fast);
  const EnemyStats heavy = enemyStats(EnemyType::Heavy);

  expect(standard.health == 30, "standard health is 30");
  expect(fast.health == 20 && fast.speed > standard.speed,
         "fast enemy is less durable and faster than standard");
  expect(heavy.health == 80 && heavy.speed < standard.speed,
         "heavy enemy is more durable and slower than standard");

  const WaveDefinition waveOne = buildWaveDefinition(1);
  const WaveDefinition waveThree = buildWaveDefinition(3);
  const WaveDefinition waveFive = buildWaveDefinition(5);
  const WaveDefinition lateWave = buildWaveDefinition(1000);

  expect(waveOne.spawnQuota == 3, "wave one has three enemies");
  expect(waveOne.eligibleTypes.size() == 1,
         "waves one and two have only standard enemies");
  expect(waveThree.eligibleTypes.size() == 2,
         "wave three introduces fast enemies");
  expect(waveFive.eligibleTypes.size() == 3,
         "wave five introduces heavy enemies");
  expect(lateWave.spawnQuota == 12, "wave quota is capped at 12");
  expect(lateWave.maxConcurrentEnemies == 8,
         "concurrent enemies are capped at eight");
  expect(nearlyEqual(lateWave.spawnIntervalSeconds, 0.5F),
         "spawn interval is floored at half a second");
}
