#ifndef LEVEL_MAP_STAGE_CONFIG_H
#define LEVEL_MAP_STAGE_CONFIG_H

namespace LevelMapInternal {

struct IcefieldStageConfig {
    int normalEnemyCount = 40;
    int eliteEnemyCount = 10;
    int spawnGridRows = 5;
    int spawnGridCols = 10;
    int bossSwitchAliveThreshold = 5;
    int normalVisibleLimit = 5;
    int eliteVisibleLimit = 2;
    int normalVisibleLimitWhenBossVisible = 4;
    int eliteVisibleLimitWhenBossVisible = 1;
};

const IcefieldStageConfig& GetIcefieldStageConfig();
int GetIcefieldTotalEnemyCount();

}  

#endif  
