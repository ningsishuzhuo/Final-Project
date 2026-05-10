#include "level_map_internal.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Msimg32.lib")

namespace LevelMapInternal {

int g_cameraX = 0;
int g_cameraY = 0;
int g_playerX = 0;
int g_playerY = 0;
int g_currentCharacterIndex = 0;
bool g_isMoving = false;
bool g_isDashing = false;
bool g_faceRight = true;
int g_moveDirX = 1;
int g_moveDirY = 0;
int g_playerHp = 50;
int g_playerMaxHp = 50;
int g_playerArmor = 0;
int g_playerMaxArmor = 0;
bool g_playerIsDead = false;
ULONGLONG g_playerDeathStartTick = 0;
bool g_gameVictory = false;
ULONGLONG g_gameVictoryStartTick = 0;
float g_playerEnergy = static_cast<float>(kPlayerMaxEnergy);
int g_playerMaxEnergy = kPlayerMaxEnergy;
ULONGLONG g_dashStartTick = 0;
ULONGLONG g_dashCooldownEndTick = 0;
ULONGLONG g_playerLastDamageTick = 0;
ULONGLONG g_playerArmorRegenTick = 0;
ULONGLONG g_lastPlayerUpdateTick = 0;
ULONGLONG g_lastAfterimageSpawnTick = 0;
bool g_ultimateShiftPressedLastFrame = false;
ULONGLONG g_characterUltimateNextCastTick[kCharacterCount] = { 0, 0, 0 };

AnimatedGif g_idleGifs[kCharacterCount];
AnimatedGif g_walkGifs[kCharacterCount];
ImageAsset g_playerDeathAssets[kCharacterCount];
ImageAsset g_particleAsset;
ImageAsset g_playerHpIconAsset;
ImageAsset g_playerArmorIconAsset;
ImageAsset g_playerEnergyIconAsset;
ImageAsset g_ultimateCooldownIconAssets[kCharacterCount];
ImageAsset g_ultimateCooldownGrayIconAssets[kCharacterCount];
ImageAsset g_energyDropAsset;
ImageAsset g_recoverPotionDropAsset;
ImageAsset g_energyPotionDropAsset;
ImageAsset g_lifePotionDropAsset;
std::vector<Projectile> g_playerProjectiles;
size_t g_playerProjectileOverflowCursor = 0;
std::vector<DashAfterimage> g_playerDashAfterimages;
AnimatedGif g_apolloSlashGif;
ApolloSlashState g_apolloSlashState;
ULONGLONG g_apolloSlashNextAvailableTick = 0;
ImageAsset g_sunUltimateSwordQiAsset;
ImageAsset g_sunCriticalHaloAsset;
std::vector<SunSwordQiState> g_sunSwordQiProjectiles;
int g_sunCriticalHitCount = 0;
ImageAsset g_moonRainArrowAsset;
ImageAsset g_moonRainAimCircleAsset;
ImageAsset g_moonUltimateFireballAsset;
ImageAsset g_moonUltimateFireballAuraAsset;
ImageAsset g_moonUltimateWaveAsset;
ImageAsset g_moonMagicCageAsset;
ImageAsset g_sunUltimateShieldAsset;
ImageAsset g_loveUltimateRecoverCircleAsset;
ImageAsset g_loveUltimateBulletAsset;
ImageAsset g_lovePurifyCircleAsset;
MoonRainChargeState g_moonRainChargeState;
MoonRainCastState g_moonRainCastState;
MoonUltimateState g_moonUltimateState;
std::vector<MoonUltimateFireballState> g_moonUltimateFireballs;
ULONGLONG g_moonUltimateFireballNextCastTick = 0;
SunUltimateState g_sunUltimateState;
LoveUltimateState g_loveUltimateState;
LovePurifyState g_lovePurifyState;
std::vector<EnergyDrop> g_energyDrops;
size_t g_energyDropOverflowCursor = 0;
std::vector<ShockwaveInstance> g_enemyShockwaves;
std::vector<EnemySpikeRow> g_enemySpikeRows;

EnemyInstance g_enemy;
GameData::EnemyKind g_enemyKind = GameData::EnemyKind::SnowApe;
GameData::EnemyCombatParams g_enemyParams = {};
AnimatedGif g_enemyIdleGif;
AnimatedGif g_enemyWalkGif;
AnimatedGif g_enemyBreakGif;
AnimatedGif g_enemySpecialAttackGif;
AnimatedGif g_enemyJumpGif;
ImageAsset g_enemyDeathAsset;
ImageAsset g_enemyBulletAsset;
ImageAsset g_enemyIceSpikeAsset;
std::vector<Projectile> g_enemyProjectiles;
size_t g_enemyProjectileOverflowCursor = 0;
int g_enemyAnimReferenceWidth = 0;
int g_enemyAnimReferenceHeight = 0;
bool g_enemySawPlayerLastFrame = false;
BattlePhase g_battlePhase = BattlePhase::NormalFight;
std::vector<IcefieldEnemy> g_icefieldEnemies;
int g_activeIcefieldEnemyIndex = -1;

RECT g_iceRegionRect;

}  
