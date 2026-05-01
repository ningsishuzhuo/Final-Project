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
extern IMAGE g_playerDeathImages[kCharacterCount];
extern bool g_playerDeathHasAlpha[kCharacterCount];
extern IMAGE g_particleImage;
extern bool g_particleHasAlpha;
extern IMAGE g_playerHpIconImage;
extern bool g_playerHpIconHasAlpha;
extern IMAGE g_playerArmorIconImage;
extern bool g_playerArmorIconHasAlpha;
extern IMAGE g_playerEnergyIconImage;
extern bool g_playerEnergyIconHasAlpha;
extern IMAGE g_ultimateCooldownIconImages[kCharacterCount];
extern bool g_ultimateCooldownIconHasAlpha[kCharacterCount];
extern IMAGE g_ultimateCooldownGrayIconImages[kCharacterCount];
extern bool g_ultimateCooldownGrayIconHasAlpha[kCharacterCount];
extern IMAGE g_energyDropImage;
extern bool g_energyDropHasAlpha;
extern IMAGE g_recoverPotionDropImage;
extern bool g_recoverPotionDropHasAlpha;
extern IMAGE g_energyPotionDropImage;
extern bool g_energyPotionDropHasAlpha;
extern IMAGE g_lifePotionDropImage;
extern bool g_lifePotionDropHasAlpha;
extern std::vector<Projectile> g_playerProjectiles;
extern size_t g_playerProjectileOverflowCursor;
extern std::vector<DashAfterimage> g_playerDashAfterimages;
extern AnimatedGif g_apolloSlashGif;
extern ApolloSlashState g_apolloSlashState;
extern ULONGLONG g_apolloSlashNextAvailableTick;
extern IMAGE g_sunUltimateSwordQiImage;
extern bool g_sunUltimateSwordQiHasAlpha;
extern IMAGE g_sunCriticalHaloImage;
extern bool g_sunCriticalHaloHasAlpha;
extern std::vector<SunSwordQiState> g_sunSwordQiProjectiles;
extern int g_sunCriticalHitCount;
extern IMAGE g_moonRainArrowImage;
extern bool g_moonRainArrowHasAlpha;
extern IMAGE g_moonRainAimCircleImage;
extern bool g_moonRainAimCircleHasAlpha;
extern IMAGE g_moonUltimateFireballImage;
extern bool g_moonUltimateFireballHasAlpha;
extern IMAGE g_moonUltimateFireballAuraImage;
extern bool g_moonUltimateFireballAuraHasAlpha;
extern IMAGE g_moonUltimateWaveImage;
extern bool g_moonUltimateWaveHasAlpha;
extern IMAGE g_moonMagicCageImage;
extern bool g_moonMagicCageHasAlpha;
extern IMAGE g_sunUltimateShieldImage;
extern bool g_sunUltimateShieldHasAlpha;
extern IMAGE g_loveUltimateRecoverCircleImage;
extern bool g_loveUltimateRecoverCircleHasAlpha;
extern IMAGE g_loveUltimateBulletImage;
extern bool g_loveUltimateBulletHasAlpha;
extern IMAGE g_lovePurifyCircleImage;
extern bool g_lovePurifyCircleHasAlpha;
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
extern IMAGE g_enemyDeathImage;
extern bool g_enemyDeathHasAlpha;
extern IMAGE g_enemyBulletImage;
extern bool g_enemyBulletHasAlpha;
extern IMAGE g_enemyIceSpikeImage;
extern bool g_enemyIceSpikeHasAlpha;
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
