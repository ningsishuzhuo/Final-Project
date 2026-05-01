#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <tchar.h>

namespace GameData {

enum class EnemyKind {
    Miner = 0,
    SnowApe = 1,
    SnowApeKing = 2
};

enum class EnemyTier {
    Boss = 0,
    Elite = 1,
    Normal = 2
};

enum class EnemyAttackMode {
    Ranged = 0,
    Melee = 1,
    Boss = 2
};

struct CharacterDefinition {
    const TCHAR* placeholderName;
    const TCHAR* displayName;
    const TCHAR* subtitle;
    const TCHAR* weaponLabel;
    const TCHAR* personLabel;
    bool usesProjectileAttack;
    int maxHp;
};

struct EnemyCombatParams {
    int hp;
    int chaseSpeed;
    int patrolSpeed;
    int bulletSpeed;
    int attackIntervalMs;
    float preferredMinDistance;
    float preferredMaxDistance;
    int repositionIntervalMs;
    int dodgeDetectRadius;
    int dodgeStep;
    int dodgeMoveSpeed;
    float dodgePredictionFrames;
    float dodgeLaneHalfWidth;
    int hitRadius;
    int chargeSpeed;
    int chargeDurationMs;
    int chargeTriggerDistance;
    int slamDurationMs;
    int shockwaveExpandSpeed;
    int shockwaveMaxRadius;
    int shockwaveHitRadius;
};

struct EnemyDefinition {
    EnemyKind kind;
    EnemyTier tier;
    EnemyAttackMode attackMode;
    const TCHAR* displayName;
    const TCHAR* idleGifPath;
    const TCHAR* walkGifPath;
    const TCHAR* deathImagePath;
    const TCHAR* bulletImagePath;
    const TCHAR* specialAttackGifPath;
    int drawWidth;
    int drawHeight;
    int bulletDrawWidth;
    int bulletDrawHeight;
    int regionPadding;
};

const CharacterDefinition& GetCharacterDefinition(int characterIndex);
EnemyCombatParams GetEnemyCombatParams(EnemyAttackMode attackMode, EnemyTier tier);
const EnemyDefinition& GetEnemyDefinition(EnemyKind kind);

}  

#endif
