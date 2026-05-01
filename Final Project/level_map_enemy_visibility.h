#ifndef LEVEL_MAP_ENEMY_VISIBILITY_H
#define LEVEL_MAP_ENEMY_VISIBILITY_H

#include <vector>

namespace LevelMapInternal {

bool IsEnemyVisibleOnScreenByState(
    const EnemyInstance& enemy,
    const GameData::EnemyDefinition& enemyDef);

bool IsBossVisibleOnScreenByState(const EnemyInstance& bossState);

void GetCurrentOnScreenEnemyLimits(bool bossVisible, int& normalLimit, int& eliteLimit);

void BuildIcefieldVisibleMask(bool bossVisible, std::vector<bool>& allowOnScreen);

bool IsIcefieldEnemyAllowedOnScreen(int enemyIndex, bool bossVisible);

bool IsIcefieldMinerWithinAttackQuota(int enemyIndex);

}  

#endif  
