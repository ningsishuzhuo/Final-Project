#ifndef LEVEL_MAP_PLAYER_STATUS_H
#define LEVEL_MAP_PLAYER_STATUS_H

namespace LevelMapInternal {
namespace PlayerStatus {

struct CharacterStatusConfig {
    int maxHp = 50;
    int maxArmor = 0;
    int maxEnergy = 200;
};

const CharacterStatusConfig& GetCharacterStatusConfig(int characterIndex);
void ApplyIncomingDamage(int incomingDamage, int& armor, int& hp);

}  
}  

#endif  
