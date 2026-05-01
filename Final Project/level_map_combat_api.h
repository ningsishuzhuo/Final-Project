#ifndef LEVEL_MAP_COMBAT_API_H
#define LEVEL_MAP_COMBAT_API_H

#include "level_map_types.h"

#include <vector>

namespace LevelMapInternal {

const ShockwaveTuning& GetShockwaveTuning(ShockwaveInstance::Variant variant);

void ApplyPlayerHit(int energyLoss);
void SpawnPlayerProjectile(int clickScreenX, int clickScreenY);
void BeginMoonRainCharge(int clickScreenX, int clickScreenY);
void ReleaseMoonRainCharge();
void SpawnMoonUltimateFireball(int clickScreenX, int clickScreenY);
void UpdateMoonRainSkill(ULONGLONG now);
void UpdateSunSwordQiProjectiles();
void SpawnEnemyProjectile(float dx, float dy);
void SpawnEnemyProjectileFrom(
    float originX,
    float originY,
    float dx,
    float dy,
    float speedScale,
    ULONGLONG nowTick,
    ULONGLONG hitDelayMs = 0ULL,
    int damage = 0);
void SpawnEnemyShockwave(float centerX, float centerY, bool alreadyHitPlayer, ShockwaveInstance::Variant variant);
void SpawnSnowApeKingSpikeRow(float dirX, float dirY);
void NotifySnowApeKingHpLossForCrossBarrage(int hpLost);
void UpdateSnowApeKingCrossBarrage(ULONGLONG now);
void UpdateProjectileArray(std::vector<Projectile>& arr, int speedGuardWidth, int speedGuardHeight);
void UpdateEnemyShockwaves(ULONGLONG now);
void UpdateEnemySpikeRows(ULONGLONG now);
void UpdateEnergyDrops();
void HandleCombatCollisions(ULONGLONG now);
void SpawnEnergyDropsOnEnemyDeath(int centerX, int centerY, const GameData::EnemyDefinition& enemyDef);
void SpawnDropsOnCrateDestroyed(int centerX, int centerY);
void TrySpawnPotionDropOnBossHpLossThreshold(int centerX, int centerY);

}  

#endif  
