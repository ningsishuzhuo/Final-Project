#ifndef LEVEL_MAP_TYPES_H
#define LEVEL_MAP_TYPES_H

#include "game_data.h"

#include <windows.h>

#include <deque>

namespace LevelMapInternal {

struct Projectile {
    float x = 0;
    float y = 0;
    float vx = 0;
    float vy = 0;
    float angleDegrees = 0;
    int damage = 1;
    GameData::EnemyKind sourceKind = GameData::EnemyKind::Miner;
    ULONGLONG hitArmedTick = 0;
    bool loveUltimateBullet = false;
    bool active = false;
};

struct DashAfterimage {
    int x = 0;
    int y = 0;
    bool faceRight = true;
    bool moving = true;
    ULONGLONG createdTick = 0;
};

struct ApolloSlashState {
    bool active = false;
    ULONGLONG startTick = 0;
    ULONGLONG endTick = 0;
    float dirX = 1.0f;
    float dirY = 0.0f;
};

struct SunSwordQiState {
    bool active = false;
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float angleDegrees = 0.0f;
    int baseDamage = 0;
};

struct MoonRainChargeState {
    bool active = false;
    float centerX = 0.0f;
    float centerY = 0.0f;
    float radius = 0.0f;
    ULONGLONG startTick = 0;
};

struct MoonRainCastState {
    bool active = false;
    float centerX = 0.0f;
    float centerY = 0.0f;
    float radius = 0.0f;
    int damagePerTick = 0;
    int energyCost = 0;
    bool damageApplied = false;
    ULONGLONG startTick = 0;
    ULONGLONG endTick = 0;
    ULONGLONG nextDamageTick = 0;
};

struct MoonUltimateState {
    bool active = false;
    ULONGLONG startTick = 0;
    ULONGLONG endTick = 0;
};

struct MoonUltimateFireballState {
    bool active = false;
    bool damageApplied = false;
    float startX = 0.0f;
    float startY = 0.0f;
    float targetX = 0.0f;
    float targetY = 0.0f;
    ULONGLONG startTick = 0;
    ULONGLONG impactTick = 0;
    ULONGLONG endTick = 0;
};

struct SunUltimateState {
    bool active = false;
    ULONGLONG startTick = 0;
    ULONGLONG endTick = 0;
};

struct LoveUltimateState {
    bool active = false;
    ULONGLONG startTick = 0;
    ULONGLONG endTick = 0;
    ULONGLONG nextHealTick = 0;
};

struct LovePurifyState {
    bool active = false;
    ULONGLONG startTick = 0;
    ULONGLONG endTick = 0;
};

struct EnergyDrop {
    enum class Kind {
        Energy,
        RecoverPotion,
        EnergyPotion,
        LifePotion
    };

    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    int value = 0;
    int settleFrames = 0;
    Kind kind = Kind::Energy;
    bool active = false;
};

struct ShockwaveInstance {
    enum class Variant {
        Normal,
        Empowered
    };

    float x = 0;
    float y = 0;
    float previousRadius = 0;
    float radius = 0;
    ULONGLONG triggerTick = 0;
    bool active = false;
    bool expanding = false;
    bool hitPlayer = false;
    Variant variant = Variant::Normal;
};

struct EnemySpikeRow {
    float startX = 0.0f;
    float startY = 0.0f;
    float dirX = 1.0f;
    float dirY = 0.0f;
    float length = 0.0f;
    int maxSpikeCount = 0;
    int revealedSpikeCount = 0;
    ULONGLONG startTick = 0;
    ULONGLONG nextSpawnTick = 0;
    ULONGLONG endTick = 0;
    bool active = false;
    bool hitPlayer = false;
};

struct EnemyInstance {
    enum class AttackState {
        Idle,
        Windup,
        Charging,
        Slamming
    };

    int x = 0;
    int y = 0;
    float exactX = 0.0f;
    float exactY = 0.0f;
    bool alive = true;
    bool moving = false;
    bool faceRight = true;
    bool dodging = false;
    bool breakStateActive = false;
    AttackState attackState = AttackState::Idle;
    bool slamImpactApplied = false;
    int hp = 1;
    int maxHp = 1;
    int roamTargetX = 0;
    int roamTargetY = 0;
    int dodgeTargetX = 0;
    int dodgeTargetY = 0;
    int chargeStopX = 0;
    int chargeStopY = 0;
    int slamTargetX = 0;
    int slamTargetY = 0;
    float chargeVx = 0.0f;
    float chargeVy = 0.0f;
    bool bossWalkPhase = true;
    bool bossPressureActive = false;
    ULONGLONG bossNextFaceFlipTick = 0;
    int bossPendingFaceDir = 0;
    int bossPendingFaceFrames = 0;
    bool bossRoaring = false;
    bool bossRoarPendingSpikeRow = false;
    float bossRoarDirX = 1.0f;
    float bossRoarDirY = 0.0f;
    ULONGLONG bossRoarEndTick = 0;
    bool bossCrossSpiralActive = false;
    int bossCrossSpiralBurstsRemaining = 0;
    float bossCrossSpiralBaseAngleDegrees = 0.0f;
    int bossCrossSpiralEmitCooldownFrames = 0;
    bool bossCrossJumping = false;
    ULONGLONG bossCrossJumpEndTick = 0;
    int bossCrossBarrageLastHpLoss = 0;
    std::deque<int> bossCrossBarrageQueue;
    ULONGLONG bossPhaseEndTick = 0;
    ULONGLONG nextRepositionTick = 0;
    ULONGLONG nextAttackTick = 0;
    ULONGLONG attackStateEndTick = 0;
    ULONGLONG moonMagicCageEndTick = 0;
};

enum class BattlePhase {
    NormalFight,
    BossFight
};

struct IcefieldEnemy {
    GameData::EnemyKind kind = GameData::EnemyKind::Miner;
    GameData::EnemyTier tier = GameData::EnemyTier::Normal;
    EnemyInstance runtime = {};
    bool sawPlayerLastFrame = false;
    int serialNumber = 0;
};

enum class MapObstacleKind {
    Rock,
    Minecart,
    BrokenMinecart,
    OreWall,
    IceWall,
    Rail,
    Crate,
    Torch
};

struct MapObstacle {
    MapObstacleKind kind = MapObstacleKind::Rock;
    float x = 0.0f;
    float y = 0.0f;
    int width = 96;
    int height = 96;
    bool solid = true;
    bool destructible = false;
    bool destroyed = false;
    int hp = 1;
};

struct ShockwaveTuning {
    ULONGLONG lockDelayMs;
    int telegraphRadius;
    int expandSpeed;
    int maxRadius;
    int hitRadius;
    COLORREF fillColor;
    COLORREF outerColor;
    COLORREF innerColor;
    COLORREF coreColor;
    COLORREF flashColor;
};

}  

#endif  
