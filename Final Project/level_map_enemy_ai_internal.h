#ifndef LEVEL_MAP_ENEMY_AI_INTERNAL_H
#define LEVEL_MAP_ENEMY_AI_INTERNAL_H

#include <windows.h>

#include "game_data.h"

namespace LevelMapInternal {

void UpdateSnowApeKingAI(
    float dx,
    float dy,
    ULONGLONG now,
    const GameData::EnemyCombatParams& params,
    const GameData::EnemyDefinition& enemyDef,
    bool attackEnabled);

}  

#endif  
