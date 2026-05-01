#include "level_map_internal.h"
#include "level_map_combat_helpers_internal.h"

namespace LevelMapInternal {

void ClampPlayerHp() {
    if (g_playerHp < 0) {
        g_playerHp = 0;
    }
    if (g_playerHp > g_playerMaxHp) {
        g_playerHp = g_playerMaxHp;
    }
}

void ClampPlayerEnergy() {
    if (g_playerEnergy < 0.0f) {
        g_playerEnergy = 0.0f;
    }
    const float maxEnergy = static_cast<float>(g_playerMaxEnergy > 0 ? g_playerMaxEnergy : kPlayerMaxEnergy);
    if (g_playerEnergy > maxEnergy) {
        g_playerEnergy = maxEnergy;
    }
}

void ClampPlayerArmor() {
    if (g_playerArmor < 0) {
        g_playerArmor = 0;
    }
    if (g_playerArmor > g_playerMaxArmor) {
        g_playerArmor = g_playerMaxArmor;
    }
}

void CompactProjectileArray(std::vector<Projectile>& arr) {
    size_t firstInactive = arr.size();
    for (size_t i = 0; i < arr.size(); ++i) {
        if (!arr[i].active) {
            firstInactive = i;
            break;
        }
    }

    if (firstInactive >= arr.size()) {
        return;
    }

    size_t writeIndex = firstInactive;
    for (size_t i = firstInactive + 1; i < arr.size(); ++i) {
        if (!arr[i].active) {
            continue;
        }
        if (writeIndex != i) {
            arr[writeIndex] = arr[i];
        }
        ++writeIndex;
    }
    arr.resize(writeIndex);
}

void PushProjectileCapped(
    std::vector<Projectile>& projectiles,
    const Projectile& projectile,
    size_t cap,
    size_t& overflowCursor) {
    if (cap == 0U) {
        return;
    }

    if (projectiles.size() < cap) {
        projectiles.push_back(projectile);
        return;
    }

    const size_t size = projectiles.size();
    if (size == 0U) {
        return;
    }

    const size_t start = overflowCursor % size;
    for (size_t offset = 0; offset < size; ++offset) {
        const size_t index = (start + offset) % size;
        if (projectiles[index].active) {
            continue;
        }

        projectiles[index] = projectile;
        overflowCursor = (index + 1U) % size;
        return;
    }

    projectiles[start] = projectile;
    overflowCursor = (start + 1U) % size;
}

}  
