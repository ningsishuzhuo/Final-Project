#include "level_map_internal.h"
#include "level_map_damage_rules.h"

namespace LevelMapInternal {
namespace {

bool UsesHeavyProjectileDamage(GameData::EnemyTier tier) {
    return tier == GameData::EnemyTier::Boss || tier == GameData::EnemyTier::Elite;
}

}  

int GetEnemyProjectileDamageByTier(GameData::EnemyTier tier) {
    return UsesHeavyProjectileDamage(tier) ? kEliteEnemyHitDamage : kNormalEnemyHitDamage;
}

int GetBossCrossRingProjectileDamage() {
    return kBossCrossRingHitDamage;
}

int GetBossCrossSpiralProjectileDamage() {
    return kBossCrossSpiralHitDamage;
}

int GetSnowApeKingSpikeRowDamage() {
    return kSnowApeKingSpikeHitDamage;
}

int GetShockwaveHitDamage() {
    return kEnemyShockwaveHitDamage;
}

}  
