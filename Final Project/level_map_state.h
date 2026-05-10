#ifndef LEVEL_MAP_STATE_H
#define LEVEL_MAP_STATE_H

#include "level_map_constants.h"
#include "level_map_types.h"
#include "render_utils.h"

#include <graphics.h>
#include <windows.h>

#include <vector>

namespace LevelMapInternal {

using RenderUtils::AnimatedGif;

struct ImageAsset {
    IMAGE image;
    bool hasAlpha = false;
};

extern int g_cameraX;
extern int g_cameraY;
extern int g_playerX;
extern int g_playerY;
extern int g_currentCharacterIndex;
extern bool g_isMoving;
extern bool g_isDashing;
extern bool g_faceRight;
extern int g_moveDirX;
extern int g_moveDirY;
extern int g_playerHp;
extern int g_playerMaxHp;
extern int g_playerArmor;
extern int g_playerMaxArmor;
extern bool g_playerIsDead;
extern ULONGLONG g_playerDeathStartTick;
extern bool g_gameVictory;
extern ULONGLONG g_gameVictoryStartTick;
extern float g_playerEnergy;
extern int g_playerMaxEnergy;
extern ULONGLONG g_dashStartTick;
extern ULONGLONG g_dashCooldownEndTick;
extern ULONGLONG g_playerLastDamageTick;
extern ULONGLONG g_playerArmorRegenTick;
extern ULONGLONG g_lastPlayerUpdateTick;
extern ULONGLONG g_lastAfterimageSpawnTick;
extern bool g_ultimateShiftPressedLastFrame;
extern ULONGLONG g_characterUltimateNextCastTick[kCharacterCount];

extern AnimatedGif g_idleGifs[kCharacterCount];
extern AnimatedGif g_walkGifs[kCharacterCount];
extern ImageAsset g_playerDeathAssets[kCharacterCount];
extern ImageAsset g_particleAsset;
extern ImageAsset g_playerHpIconAsset;
extern ImageAsset g_playerArmorIconAsset;
extern ImageAsset g_playerEnergyIconAsset;
extern ImageAsset g_ultimateCooldownIconAssets[kCharacterCount];
extern ImageAsset g_ultimateCooldownGrayIconAssets[kCharacterCount];
extern ImageAsset g_energyDropAsset;
extern ImageAsset g_recoverPotionDropAsset;
extern ImageAsset g_energyPotionDropAsset;
extern ImageAsset g_lifePotionDropAsset;
extern std::vector<Projectile> g_playerProjectiles;
extern size_t g_playerProjectileOverflowCursor;
extern std::vector<DashAfterimage> g_playerDashAfterimages;
extern AnimatedGif g_apolloSlashGif;
extern ApolloSlashState g_apolloSlashState;
extern ULONGLONG g_apolloSlashNextAvailableTick;
extern ImageAsset g_sunUltimateSwordQiAsset;
extern ImageAsset g_sunCriticalHaloAsset;
extern std::vector<SunSwordQiState> g_sunSwordQiProjectiles;
extern int g_sunCriticalHitCount;
extern ImageAsset g_moonRainArrowAsset;
extern ImageAsset g_moonRainAimCircleAsset;
extern ImageAsset g_moonUltimateFireballAsset;
extern ImageAsset g_moonUltimateFireballAuraAsset;
extern ImageAsset g_moonUltimateWaveAsset;
extern ImageAsset g_moonMagicCageAsset;
extern ImageAsset g_sunUltimateShieldAsset;
extern ImageAsset g_loveUltimateRecoverCircleAsset;
extern ImageAsset g_loveUltimateBulletAsset;
extern ImageAsset g_lovePurifyCircleAsset;
extern MoonRainChargeState g_moonRainChargeState;
extern MoonRainCastState g_moonRainCastState;
extern MoonUltimateState g_moonUltimateState;
extern std::vector<MoonUltimateFireballState> g_moonUltimateFireballs;
extern ULONGLONG g_moonUltimateFireballNextCastTick;
extern SunUltimateState g_sunUltimateState;
extern LoveUltimateState g_loveUltimateState;
extern LovePurifyState g_lovePurifyState;
extern std::vector<EnergyDrop> g_energyDrops;
extern size_t g_energyDropOverflowCursor;
extern std::vector<ShockwaveInstance> g_enemyShockwaves;
extern std::vector<EnemySpikeRow> g_enemySpikeRows;

extern EnemyInstance g_enemy;
extern GameData::EnemyKind g_enemyKind;
extern GameData::EnemyCombatParams g_enemyParams;
extern AnimatedGif g_enemyIdleGif;
extern AnimatedGif g_enemyWalkGif;
extern AnimatedGif g_enemyBreakGif;
extern AnimatedGif g_enemySpecialAttackGif;
extern AnimatedGif g_enemyJumpGif;
extern ImageAsset g_enemyDeathAsset;
extern ImageAsset g_enemyBulletAsset;
extern ImageAsset g_enemyIceSpikeAsset;
extern std::vector<Projectile> g_enemyProjectiles;
extern size_t g_enemyProjectileOverflowCursor;
extern int g_enemyAnimReferenceWidth;
extern int g_enemyAnimReferenceHeight;
extern bool g_enemySawPlayerLastFrame;
extern BattlePhase g_battlePhase;
extern std::vector<IcefieldEnemy> g_icefieldEnemies;
extern int g_activeIcefieldEnemyIndex;

extern RECT g_iceRegionRect;

}  

#endif  
