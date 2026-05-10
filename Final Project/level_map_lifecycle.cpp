#include "level_map.h"
#include "level_map_internal.h"
#include "level_map_battle_flow.h"
#include "character_select_module.h"
#include "level_map_stage_config.h"
#include "test_toggles.h"

#include <cstdlib>

using namespace LevelMapInternal;

namespace LevelMap {
namespace {

bool CanReturnFromResultOverlay(ULONGLONG now) {
    if (g_playerIsDead) {
        return g_playerDeathStartTick > 0 &&
            now >= g_playerDeathStartTick + kResultReturnDelayMs;
    }
    if (g_gameVictory) {
        return g_gameVictoryStartTick > 0 &&
            now >= g_gameVictoryStartTick + kResultReturnDelayMs;
    }
    return false;
}

}  

void Initialize() {
    std::srand(static_cast<unsigned int>(RenderUtils::NowTickMs()));
    TestToggles::ResetDefaults();
    RenderUtils::AcquireGdiplus();
    g_playerProjectiles.reserve(kPlayerProjectileMaxCount);
    g_enemyProjectiles.reserve(kEnemyBulletMaxCount);
    g_energyDrops.reserve(kEnergyDropMaxCount);
    g_enemyShockwaves.reserve(kEnemyShockwaveMaxCount);
    g_enemySpikeRows.reserve(8);
    g_playerDashAfterimages.reserve(kDashAfterimageReserveCount);
    g_icefieldEnemies.reserve(static_cast<size_t>(GetIcefieldTotalEnemyCount()));
    BuildRegions();
    LoadAllCharacterGifs();
    LoadAllCharacterDeathImages();
    LoadParticleImage();
    LoadPlayerHpIconImage();
    LoadPlayerArmorIconImage();
    LoadPlayerEnergyIconImage();
    LoadEnergyDropImage();
    LoadMapObstacleAssets();
    ResetMapObstacles();
    ResetCamera();
    LoadEnemyAssets();
}

void Shutdown() {
    FreeAllCharacterGifs();
    FreeAllCharacterDeathImages();
    FreeEnemyAssets();
    g_playerProjectiles.clear();
    g_playerProjectileOverflowCursor = 0;
    g_enemyProjectiles.clear();
    g_enemyProjectileOverflowCursor = 0;
    g_energyDrops.clear();
    g_energyDropOverflowCursor = 0;
    g_enemyShockwaves.clear();
    g_enemySpikeRows.clear();
    g_icefieldEnemies.clear();
    g_activeIcefieldEnemyIndex = -1;
    g_particleAsset.image.Resize(0, 0);
    g_particleAsset.hasAlpha = false;
    g_playerHpIconAsset.image.Resize(0, 0);
    g_playerHpIconAsset.hasAlpha = false;
    g_playerArmorIconAsset.image.Resize(0, 0);
    g_playerArmorIconAsset.hasAlpha = false;
    g_playerEnergyIconAsset.image.Resize(0, 0);
    g_playerEnergyIconAsset.hasAlpha = false;
    g_energyDropAsset.image.Resize(0, 0);
    g_energyDropAsset.hasAlpha = false;
    g_recoverPotionDropAsset.image.Resize(0, 0);
    g_recoverPotionDropAsset.hasAlpha = false;
    g_energyPotionDropAsset.image.Resize(0, 0);
    g_energyPotionDropAsset.hasAlpha = false;
    g_lifePotionDropAsset.image.Resize(0, 0);
    g_lifePotionDropAsset.hasAlpha = false;
    FreeMapObstacleAssets();
    RenderUtils::ReleaseGdiplus();
}

void ResetCamera() {
    g_currentCharacterIndex = CharacterSelectModule::GetSelectedCharacterIndex();
    if (g_currentCharacterIndex < 0 || g_currentCharacterIndex >= kCharacterCount) {
        g_currentCharacterIndex = 0;
    }

    g_playerX = kLevelMapWidth / 2;
    g_playerY = kLevelMapHeight / 2;
    g_isMoving = false;
    g_isDashing = false;
    g_faceRight = true;
    g_moveDirX = 1;
    g_moveDirY = 0;
    const PlayerStatus::CharacterStatusConfig& statusConfig =
        PlayerStatus::GetCharacterStatusConfig(g_currentCharacterIndex);
    g_playerMaxHp = statusConfig.maxHp > 0 ? statusConfig.maxHp : 50;
    g_playerHp = g_playerMaxHp;
    g_playerMaxArmor = statusConfig.maxArmor > 0 ? statusConfig.maxArmor : 0;
    g_playerArmor = g_playerMaxArmor;
    g_playerMaxEnergy = statusConfig.maxEnergy > 0 ? statusConfig.maxEnergy : kPlayerMaxEnergy;
    g_playerIsDead = false;
    g_playerDeathStartTick = 0;
    g_gameVictory = false;
    g_gameVictoryStartTick = 0;
    g_playerEnergy = static_cast<float>(g_playerMaxEnergy);
    g_dashStartTick = 0;
    g_dashCooldownEndTick = 0;
    g_lastPlayerUpdateTick = RenderUtils::NowTickMs();
    g_playerLastDamageTick = g_lastPlayerUpdateTick;
    g_playerArmorRegenTick = 0;
    g_lastAfterimageSpawnTick = 0;
    g_ultimateShiftPressedLastFrame = false;
    for (int i = 0; i < kCharacterCount; ++i) {
        g_characterUltimateNextCastTick[i] = 0;
    }
    g_apolloSlashState = {};
    g_apolloSlashNextAvailableTick = 0;
    g_sunSwordQiProjectiles.clear();
    g_sunCriticalHitCount = 0;
    g_moonRainChargeState = {};
    g_moonRainCastState = {};
    g_moonUltimateState = {};
    g_moonUltimateFireballs.clear();
    g_moonUltimateFireballNextCastTick = 0;
    g_sunUltimateState = {};
    g_loveUltimateState = {};
    g_lovePurifyState = {};
    g_enemySawPlayerLastFrame = false;

    g_playerProjectiles.clear();
    g_playerProjectileOverflowCursor = 0;
    g_playerDashAfterimages.clear();
    g_enemyProjectiles.clear();
    g_enemyProjectileOverflowCursor = 0;
    g_energyDrops.clear();
    g_energyDropOverflowCursor = 0;
    g_enemyShockwaves.clear();
    g_enemySpikeRows.clear();
    g_icefieldEnemies.clear();
    g_activeIcefieldEnemyIndex = -1;
    g_battlePhase = BattlePhase::NormalFight;
    ResetMapObstacles();
    SpawnEnemyInIceRegion();
    UpdateCameraToPlayer();
}

void HandleMouseButtonDown(int x, int y) {
    if (g_playerIsDead || g_gameVictory) {
        return;
    }
    if (g_currentCharacterIndex == AssetPaths::CHARACTER_MOON) {
        const ULONGLONG now = RenderUtils::NowTickMs();
        if (g_moonUltimateState.active && now < g_moonUltimateState.endTick) {
            SpawnMoonUltimateFireball(x, y);
            return;
        }
        BeginMoonRainCharge(x, y);
        return;
    }
    SpawnPlayerProjectile(x, y);
}

void HandleMouseButtonUp(int x, int y) {
    (void)x;
    (void)y;
    if (g_playerIsDead || g_gameVictory) {
        return;
    }
    if (g_currentCharacterIndex == AssetPaths::CHARACTER_MOON) {
        ReleaseMoonRainCharge();
    }
}

void HandleMouseClick(int x, int y) {
    HandleMouseButtonDown(x, y);
}

bool HandleAnyKeyDown() {
    if (!CanReturnFromResultOverlay(RenderUtils::NowTickMs())) {
        return false;
    }

    ResetCamera();
    return true;
}

bool IsBossBattleActive() {
    return g_battlePhase == BattlePhase::BossFight;
}

void Draw() {
    const ULONGLONG now = RenderUtils::NowTickMs();
    if (g_playerIsDead || g_gameVictory) {
        RenderLevelFrame(now);
        return;
    }

    UpdatePlayerByKeyboard(now);
    UpdateEnemyAI(now);
    ResolvePlayerEnemyCollision();
    UpdateCameraToPlayer();

    UpdateProjectileArray(g_playerProjectiles, 220, 220);
    UpdateSunSwordQiProjectiles();
    const bool isBossCrossBarrage = EnemyDef().kind == GameData::EnemyKind::SnowApeKing;
    const int enemyProjectileGuard = isBossCrossBarrage ? 1400 : 260;
    UpdateProjectileArray(g_enemyProjectiles, enemyProjectileGuard, enemyProjectileGuard);
    UpdateEnemyShockwaves(now);
    UpdateEnemySpikeRows(now);
    UpdateEnergyDrops();
    UpdateMoonRainSkill(now);

    HandleCombatCollisions(now);
    if (g_battlePhase == BattlePhase::BossFight &&
        !g_enemy.alive &&
        CountAliveIcefieldEnemies() <= 0) {
        g_gameVictory = true;
        g_gameVictoryStartTick = now;
        g_isMoving = false;
        g_isDashing = false;
    }
    UpdateSnowApeKingCrossBarrage(now);
    RenderLevelFrame(now);
}

}  
