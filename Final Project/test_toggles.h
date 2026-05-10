#ifndef TEST_TOGGLES_H
#define TEST_TOGGLES_H

namespace TestToggles {

struct RuntimeFlags {
    bool infinitePlayerEnergy = false;
    bool startNormalBattleWithSixEnemies = false;
    bool showAuxiliaryHudText = false;
};

const RuntimeFlags& Get();
RuntimeFlags& Mutable();
void ResetDefaults();

}  

#endif  
