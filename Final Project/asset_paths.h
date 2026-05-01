#ifndef ASSET_PATHS_H
#define ASSET_PATHS_H

#include <tchar.h>

namespace AssetPaths {

enum CharacterId {
    CHARACTER_MOON = 0,
    CHARACTER_SUN = 1,
    CHARACTER_LOVE = 2,
    CHARACTER_COUNT = 3
};

const TCHAR* GetAssetRoot();
const TCHAR* GetTypeFontPath();

const TCHAR* GetLogoPath(int index);
const TCHAR* GetUiBackgroundPath();
const TCHAR* GetMenuButtonIconPath(int index);
const TCHAR* GetMainMenuMusicPath();
const TCHAR* GetCharacterSelectMusicPath();
const TCHAR* GetNormalBattleMusicPath();
const TCHAR* GetBossBattleMusicPath();

const TCHAR* GetCharacterPortraitPath(int characterIndex);
const TCHAR* GetCharacterMoonPath(int characterIndex);
const TCHAR* GetCharacterSelectGifPath(int characterIndex);
const TCHAR* GetCharacterIdleGifPath(int characterIndex);
const TCHAR* GetCharacterWalkGifPath(int characterIndex);
const TCHAR* GetCharacterDeathImagePath(int characterIndex);
const TCHAR* GetApolloSlashGifPath();
const TCHAR* GetSunUltimateSwordQiPath();
const TCHAR* GetSunCriticalHaloPath();
const TCHAR* GetMoonRainArrowPath();
const TCHAR* GetMoonRainAimCirclePath();
const TCHAR* GetMoonUltimateFireballPath();
const TCHAR* GetMoonUltimateFireballAuraPath();
const TCHAR* GetMoonUltimateWavePath();
const TCHAR* GetMoonMagicCagePath();
const TCHAR* GetSunUltimateShieldPath();
const TCHAR* GetLoveUltimateRecoverCirclePath();
const TCHAR* GetLoveUltimateBulletPath();
const TCHAR* GetLovePurifyCirclePath();
const TCHAR* GetUltimateCooldownIconPath(int characterIndex);

const TCHAR* GetLoveParticlePath();
const TCHAR* GetMinerEnemyIdlePath();
const TCHAR* GetMinerEnemyWalkPath();
const TCHAR* GetMinerEnemyDeathPath();
const TCHAR* GetMinerEnemyBulletPath();
const TCHAR* GetSnowApeEnemyIdlePath();
const TCHAR* GetSnowApeEnemyWalkPath();
const TCHAR* GetSnowApeEnemyDeathPath();
const TCHAR* GetSnowApeShockwavePath();
const TCHAR* GetSnowApeKingEnemyIdlePath();
const TCHAR* GetSnowApeKingEnemyWalkPath();
const TCHAR* GetSnowApeKingEnemyDeathPath();
const TCHAR* GetSnowApeKingEnemyBulletPath();
const TCHAR* GetSnowApeKingEnemyJumpPath();
const TCHAR* GetSnowApeKingEnemyRoarPath();
const TCHAR* GetSnowApeKingIceSpikePath();
const TCHAR* GetIcefieldWallPath();
const TCHAR* GetIcefieldRockPath();
const TCHAR* GetIcefieldMinecartPath();
const TCHAR* GetIcefieldBrokenMinecartPath();
const TCHAR* GetIcefieldOreWallPath();
const TCHAR* GetIcefieldRailPath();
const TCHAR* GetIcefieldCratePath();
const TCHAR* GetIcefieldTorchPath();
const TCHAR* GetEliteBreakStatePath();
const TCHAR* GetPlayerHpIconPath();
const TCHAR* GetPlayerArmorIconPath();
const TCHAR* GetPlayerEnergyIconPath();
const TCHAR* GetEnergyDropIconPath();
const TCHAR* GetRecoverPotionDropIconPath();
const TCHAR* GetEnergyPotionDropIconPath();
const TCHAR* GetLifePotionDropIconPath();

}  

#endif
