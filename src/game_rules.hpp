#pragma once

#include <vector>

enum class EnemyType {
  Standard,
  Fast,
  Heavy,
};

struct EnemyStats {
  int health;
  float speed;
  float contactDamagePerSecond;
};

struct WaveDefinition {
  int spawnQuota;
  int maxConcurrentEnemies;
  float spawnIntervalSeconds;
  std::vector<EnemyType> eligibleTypes;
};

EnemyStats enemyStats(EnemyType type);
WaveDefinition buildWaveDefinition(int waveNumber);
