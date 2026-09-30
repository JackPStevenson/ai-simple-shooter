#include "game_rules.hpp"

#include <algorithm>

namespace {

constexpr int kMaximumWaveQuota = 12;
constexpr int kMaximumConcurrentEnemies = 8;
constexpr float kMinimumSpawnIntervalSeconds = 0.5F;

}  // namespace

EnemyStats enemyStats(EnemyType type) {
  switch (type) {
    case EnemyType::Standard:
      return {30, 2.0F, 10.0F};
    case EnemyType::Fast:
      return {20, 3.5F, 8.0F};
    case EnemyType::Heavy:
      return {80, 1.2F, 16.0F};
  }

  return {0, 0.0F, 0.0F};
}

WaveDefinition buildWaveDefinition(int waveNumber) {
  const int safeWaveNumber = std::max(waveNumber, 1);
  WaveDefinition wave{
      std::min(2 + safeWaveNumber, kMaximumWaveQuota),
      kMaximumConcurrentEnemies,
      std::max(1.0F - 0.1F * static_cast<float>(safeWaveNumber - 1),
               kMinimumSpawnIntervalSeconds),
      {EnemyType::Standard},
  };

  if (safeWaveNumber >= 3) {
    wave.eligibleTypes.push_back(EnemyType::Fast);
  }
  if (safeWaveNumber >= 5) {
    wave.eligibleTypes.push_back(EnemyType::Heavy);
  }

  return wave;
}
