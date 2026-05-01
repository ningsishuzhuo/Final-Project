#ifndef LEVEL_MAP_COMBAT_HELPERS_INTERNAL_H
#define LEVEL_MAP_COMBAT_HELPERS_INTERNAL_H

#include "level_map_types.h"

#include <cstddef>
#include <vector>

namespace LevelMapInternal {

void ClampPlayerHp();
void ClampPlayerEnergy();
void ClampPlayerArmor();
void CompactProjectileArray(std::vector<Projectile>& arr);
void PushProjectileCapped(
    std::vector<Projectile>& projectiles,
    const Projectile& projectile,
    size_t cap,
    size_t& overflowCursor);

}  

#endif  
