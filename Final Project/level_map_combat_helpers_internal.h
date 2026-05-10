#ifndef LEVEL_MAP_COMBAT_HELPERS_INTERNAL_H
#define LEVEL_MAP_COMBAT_HELPERS_INTERNAL_H

#include "level_map_types.h"

#include <vector>

namespace LevelMapInternal {

void ClampPlayerHp();
void ClampPlayerEnergy();
void ClampPlayerArmor();
void CompactProjectileArray(std::vector<Projectile>& arr);
void PushPlayerProjectileCapped(const Projectile& projectile);
void PushEnemyProjectileCapped(const Projectile& projectile);

}  

#endif  
