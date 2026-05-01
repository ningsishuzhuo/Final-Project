#include "game_data.h"

#include "asset_paths.h"

namespace GameData {
namespace {

constexpr int kEnemyCount = 3;

constexpr CharacterDefinition Character(
    const TCHAR* name,
    const TCHAR* subtitle,
    bool usesProjectileAttack,
    int maxHp) {
    return { name, name, subtitle, _T("武器"), _T("人物"), usesProjectileAttack, maxHp };
}

constexpr EnemyCombatParams Combat(
    int hp,
    int chaseSpeed,
    int patrolSpeed,
    int bulletSpeed,
    int attackIntervalMs,
    float preferredMinDistance,
    float preferredMaxDistance,
    int repositionIntervalMs,
    int dodgeDetectRadius,
    int dodgeStep,
    int dodgeMoveSpeed,
    float dodgePredictionFrames,
    float dodgeLaneHalfWidth,
    int hitRadius,
    int chargeSpeed,
    int chargeDurationMs,
    int chargeTriggerDistance,
    int slamDurationMs,
    int shockwaveExpandSpeed,
    int shockwaveMaxRadius,
    int shockwaveHitRadius) {
    return {
        hp,
        chaseSpeed,
        patrolSpeed,
        bulletSpeed,
        attackIntervalMs,
        preferredMinDistance,
        preferredMaxDistance,
        repositionIntervalMs,
        dodgeDetectRadius,
        dodgeStep,
        dodgeMoveSpeed,
        dodgePredictionFrames,
        dodgeLaneHalfWidth,
        hitRadius,
        chargeSpeed,
        chargeDurationMs,
        chargeTriggerDistance,
        slamDurationMs,
        shockwaveExpandSpeed,
        shockwaveMaxRadius,
        shockwaveHitRadius
    };
}

constexpr CharacterDefinition kCharacterDefinitions[AssetPaths::CHARACTER_COUNT] = {
    Character(_T("月神"), _T("阿尔忒弥斯"), false, 50),
    Character(_T("太阳神"), _T("阿波罗"), false, 10),
    Character(_T("爱神"), _T("阿芙洛狄忒"), true, 50)
};

constexpr EnemyCombatParams kRangedEnemyCombatParams = Combat(
    8, 1, 1, 7, 2100, 340.0f, 620.0f, 850, 320, 12, 4, 12.0f, 30.0f, 42,
    0, 0, 0, 0, 0, 0, 0);

constexpr EnemyCombatParams kMeleeEnemyCombatParams = Combat(
    8, 3, 2, 0, 1650, 0.0f, 220.0f, 560, 260, 10, 3, 10.0f, 26.0f, 56,
    5, 900, 220, 680, 7, 92, 20);

constexpr EnemyCombatParams kBossEnemyCombatParams = Combat(
    20, 3, 2, 9, 700, 260.0f, 540.0f, 600, 380, 14, 5, 13.0f, 34.0f, 64,
    18, 1500, 280, 700, 20, 220, 58);

EnemyDefinition Enemy(
    EnemyKind kind,
    EnemyTier tier,
    EnemyAttackMode attackMode,
    const TCHAR* displayName,
    const TCHAR* idleGifPath,
    const TCHAR* walkGifPath,
    const TCHAR* deathImagePath,
    const TCHAR* bulletImagePath,
    const TCHAR* specialAttackGifPath,
    int drawSize,
    int bulletDrawWidth,
    int bulletDrawHeight,
    int regionPadding) {
    return {
        kind,
        tier,
        attackMode,
        displayName,
        idleGifPath,
        walkGifPath,
        deathImagePath,
        bulletImagePath,
        specialAttackGifPath,
        drawSize,
        drawSize,
        bulletDrawWidth,
        bulletDrawHeight,
        regionPadding
    };
}

const EnemyDefinition kEnemyDefinitions[kEnemyCount] = {
    Enemy(
        EnemyKind::Miner,
        EnemyTier::Normal,
        EnemyAttackMode::Ranged,
        _T("矿工"),
        AssetPaths::GetMinerEnemyIdlePath(),
        AssetPaths::GetMinerEnemyWalkPath(),
        AssetPaths::GetMinerEnemyDeathPath(),
        AssetPaths::GetMinerEnemyBulletPath(),
        nullptr,
        96,
        34,
        18,
        36),
    Enemy(
        EnemyKind::SnowApe,
        EnemyTier::Elite,
        EnemyAttackMode::Melee,
        _T("大雪猿"),
        AssetPaths::GetSnowApeEnemyIdlePath(),
        AssetPaths::GetSnowApeEnemyWalkPath(),
        AssetPaths::GetSnowApeEnemyDeathPath(),
        nullptr,
        AssetPaths::GetSnowApeShockwavePath(),
        128,
        0,
        0,
        48),
    Enemy(
        EnemyKind::SnowApeKing,
        EnemyTier::Boss,
        EnemyAttackMode::Boss,
        _T("雪山大猿王"),
        AssetPaths::GetSnowApeKingEnemyIdlePath(),
        AssetPaths::GetSnowApeKingEnemyWalkPath(),
        AssetPaths::GetSnowApeKingEnemyDeathPath(),
        AssetPaths::GetSnowApeKingEnemyBulletPath(),
        AssetPaths::GetSnowApeKingEnemyRoarPath(),
        180,
        44,
        44,
        64)
};

constexpr bool IsValidCharacterIndex(int index) {
    return index >= 0 && index < AssetPaths::CHARACTER_COUNT;
}

constexpr bool IsValidEnemyIndex(int index) {
    return index >= 0 && index < kEnemyCount;
}

const EnemyCombatParams& BaseCombatParams(EnemyAttackMode attackMode) {
    switch (attackMode) {
    case EnemyAttackMode::Ranged:
        return kRangedEnemyCombatParams;
    case EnemyAttackMode::Melee:
        return kMeleeEnemyCombatParams;
    case EnemyAttackMode::Boss:
        return kBossEnemyCombatParams;
    }
    return kRangedEnemyCombatParams;
}

int HpMultiplier(EnemyTier tier) {
    switch (tier) {
    case EnemyTier::Boss:
        return 4;
    case EnemyTier::Elite:
        return 2;
    case EnemyTier::Normal:
        return 1;
    }
    return 1;
}

}  

const CharacterDefinition& GetCharacterDefinition(int characterIndex) {
    if (!IsValidCharacterIndex(characterIndex)) {
        return kCharacterDefinitions[0];
    }
    return kCharacterDefinitions[characterIndex];
}

EnemyCombatParams GetEnemyCombatParams(EnemyAttackMode attackMode, EnemyTier tier) {
    EnemyCombatParams params = BaseCombatParams(attackMode);
    params.hp *= HpMultiplier(tier);
    return params;
}

const EnemyDefinition& GetEnemyDefinition(EnemyKind kind) {
    const int index = static_cast<int>(kind);
    if (!IsValidEnemyIndex(index)) {
        return kEnemyDefinitions[0];
    }
    return kEnemyDefinitions[index];
}

}  
