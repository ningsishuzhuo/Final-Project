#include "level_map_internal.h"

namespace LevelMapInternal {

void ResetEnemyCombatRuntimeState() {
    g_enemy.moving = false;
    g_enemy.dodging = false;
    g_enemy.attackState = EnemyInstance::AttackState::Idle;
    g_enemy.slamImpactApplied = false;
    g_enemy.chargeVx = 0.0f;
    g_enemy.chargeVy = 0.0f;
}

void ResetEnemyBossRuntimeState() {
    g_enemy.bossRoaring = false;
    g_enemy.bossRoarPendingSpikeRow = false;
    g_enemy.bossRoarDirX = g_enemy.faceRight ? 1.0f : -1.0f;
    g_enemy.bossRoarDirY = 0.0f;
    g_enemy.bossRoarEndTick = 0;
    g_enemy.bossCrossSpiralActive = false;
    g_enemy.bossCrossSpiralBurstsRemaining = 0;
    g_enemy.bossCrossSpiralBaseAngleDegrees = 0.0f;
    g_enemy.bossCrossSpiralEmitCooldownFrames = 0;
    g_enemy.bossCrossJumping = false;
    g_enemy.bossCrossJumpEndTick = 0;
    g_enemy.bossCrossBarrageLastHpLoss = 0;
    g_enemy.bossCrossBarrageQueue.clear();
    g_enemy.bossPendingFaceDir = 0;
    g_enemy.bossPendingFaceFrames = 0;
}

void MarkEnemyDefeated() {
    g_enemy.alive = false;
    g_enemy.hp = 0;
    ResetEnemyCombatRuntimeState();
    ResetEnemyBossRuntimeState();

    if (g_battlePhase == BattlePhase::NormalFight &&
        g_activeIcefieldEnemyIndex >= 0 &&
        g_activeIcefieldEnemyIndex < static_cast<int>(g_icefieldEnemies.size())) {
        IcefieldEnemy& activeEnemy = g_icefieldEnemies[static_cast<size_t>(g_activeIcefieldEnemyIndex)];
        activeEnemy.runtime = g_enemy;
        activeEnemy.runtime.alive = false;
        activeEnemy.runtime.hp = 0;
    }
}

}  
