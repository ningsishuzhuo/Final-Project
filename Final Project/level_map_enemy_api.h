#ifndef LEVEL_MAP_ENEMY_API_H
#define LEVEL_MAP_ENEMY_API_H

#include "level_map_types.h"

namespace LevelMapInternal {

void SetEnemyTier(GameData::EnemyTier tier);
const GameData::EnemyCombatParams& EnemyParams();
GameData::EnemyCombatParams EnemyParamsForCurrentEnemy(ULONGLONG now);
const GameData::EnemyDefinition& EnemyDef();

void ResolvePlayerEnemyCollision();
void RefreshEnemyGridPosition();
void SetEnemyExactPosition(float x, float y);
void OffsetEnemyPosition(float deltaX, float deltaY);
float GetEnemyMoveSpeed(float baseSpeed);
void UpdateEnemyFacingTowardPlayer();
void ClampPointToIceRegion(int& x, int& y);
bool MoveEnemyTowardPoint(int targetX, int targetY, float maxStep);

void SpawnEnemyInIceRegion();
void ResetEnemyCombatRuntimeState();
void ResetEnemyBossRuntimeState();
void MarkEnemyDefeated();
void StartIcefieldBossBattle(ULONGLONG now);
bool IsIcefieldNormalBattleActive();
void GetIcefieldRemainingEnemyCounts(int& normalCount, int& eliteCount);
void SpawnIcefieldNormalWave(ULONGLONG now);
bool UpdateIcefieldNormalWaveLifecycle();
int FindNearestAliveIcefieldEnemyIndex();
void UpdateIcefieldEnemyWaveAI(ULONGLONG now);
void UpdateEnemyAI(ULONGLONG now);

void LoadEnemyAssets();
void FreeEnemyAssets();

}  

#endif  
