#ifndef LEVEL_MAP_BATTLE_FLOW_H
#define LEVEL_MAP_BATTLE_FLOW_H

namespace LevelMapInternal {

int CountAliveIcefieldEnemies();
void ReduceInitialNormalWaveToSixEnemies();
void ActivateIcefieldEnemy(int enemyIndex);
void ActivateNearestAliveIcefieldEnemy();
void InitializeIcefieldNormalBattle(ULONGLONG now);
void UpdateIcefieldWaveRuntime(ULONGLONG now);
bool UpdateBossBattleWaveAndRestoreBoss(ULONGLONG now);

}  

#endif  
