#ifndef LEVEL_MAP_DAMAGE_RULES_H
#define LEVEL_MAP_DAMAGE_RULES_H

#include "game_data.h"

namespace LevelMapInternal {

int GetEnemyProjectileDamageByTier(GameData::EnemyTier tier);
int GetBossCrossRingProjectileDamage();
int GetBossCrossSpiralProjectileDamage();
int GetSnowApeKingSpikeRowDamage();
int GetShockwaveHitDamage();

}  

#endif  
