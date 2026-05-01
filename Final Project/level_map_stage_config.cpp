#include "level_map_stage_config.h"

namespace LevelMapInternal {
namespace {

const IcefieldStageConfig kIcefieldStageConfig = {};

}  

const IcefieldStageConfig& GetIcefieldStageConfig() {
    return kIcefieldStageConfig;
}

int GetIcefieldTotalEnemyCount() {
    const IcefieldStageConfig& config = GetIcefieldStageConfig();
    return config.normalEnemyCount + config.eliteEnemyCount;
}

}  
