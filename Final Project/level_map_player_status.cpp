#include "level_map_player_status.h"

#include "asset_paths.h"

namespace LevelMapInternal {
namespace PlayerStatus {
namespace {

constexpr CharacterStatusConfig kCharacterStatusConfigs[AssetPaths::CHARACTER_COUNT] = {
    { 15, 6, 150 },   
    { 5, 8, 100 },    
    { 10, 6, 250 }    
};

}  

const CharacterStatusConfig& GetCharacterStatusConfig(int characterIndex) {
    if (characterIndex < 0 || characterIndex >= AssetPaths::CHARACTER_COUNT) {
        return kCharacterStatusConfigs[0];
    }
    return kCharacterStatusConfigs[characterIndex];
}

void ApplyIncomingDamage(int incomingDamage, int& armor, int& hp) {
    if (incomingDamage <= 0 || hp <= 0) {
        return;
    }

    int remain = incomingDamage;
    if (armor > 0) {
        const int absorbed = (armor < remain) ? armor : remain;
        armor -= absorbed;
        remain -= absorbed;
    }

    if (remain > 0) {
        hp -= remain;
    }

    if (armor < 0) {
        armor = 0;
    }
    if (hp < 0) {
        hp = 0;
    }
}

}  
}  
