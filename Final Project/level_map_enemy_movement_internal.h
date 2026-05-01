#ifndef LEVEL_MAP_ENEMY_MOVEMENT_INTERNAL_H
#define LEVEL_MAP_ENEMY_MOVEMENT_INTERNAL_H

#include <windows.h>

#include "game_data.h"

namespace LevelMapInternal {

void ClampEnemyToIceRegion(const GameData::EnemyDefinition& enemyDef);
void UpdateEnemyRoamTarget(const GameData::EnemyCombatParams& params);
void ApplyMinerSeparationForce(float& moveX, float& moveY);
bool UpdateEnemyDodgeMovement(
    const GameData::EnemyCombatParams& params,
    const GameData::EnemyDefinition& enemyDef);
bool TryEnemyDodgePlayerProjectile(
    const GameData::EnemyCombatParams& params,
    const GameData::EnemyDefinition& enemyDef);

}  

#endif  
