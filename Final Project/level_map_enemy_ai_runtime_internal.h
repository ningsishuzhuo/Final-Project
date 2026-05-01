#ifndef LEVEL_MAP_ENEMY_AI_RUNTIME_INTERNAL_H
#define LEVEL_MAP_ENEMY_AI_RUNTIME_INTERNAL_H

#include <windows.h>

#include "game_data.h"

namespace LevelMapInternal {

void UpdateRangedEnemyAI(
    float dx,
    float dy,
    float distSquared,
    ULONGLONG now,
    const GameData::EnemyCombatParams& params,
    const GameData::EnemyDefinition& enemyDef,
    bool attackEnabled);

void UpdateMeleeEnemyAI(
    float dx,
    float dy,
    float distSquared,
    ULONGLONG now,
    const GameData::EnemyCombatParams& params,
    const GameData::EnemyDefinition& enemyDef,
    bool attackEnabled);

}  

#endif  
