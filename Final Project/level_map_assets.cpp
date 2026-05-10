#include "level_map_internal.h"

namespace LevelMapInternal {
namespace {

int GetGifSourceWidth(const AnimatedGif& gif) {
    return gif.IsLoaded() ? static_cast<int>(gif.image->GetWidth()) : 0;
}

int GetGifSourceHeight(const AnimatedGif& gif) {
    return gif.IsLoaded() ? static_cast<int>(gif.image->GetHeight()) : 0;
}

void RefreshEnemyAnimationReferenceSize() {
    g_enemyAnimReferenceWidth = 0;
    g_enemyAnimReferenceHeight = 0;

    auto includeSize = [](const AnimatedGif& gif) {
        const int width = GetGifSourceWidth(gif);
        const int height = GetGifSourceHeight(gif);
        if (width > g_enemyAnimReferenceWidth) {
            g_enemyAnimReferenceWidth = width;
        }
        if (height > g_enemyAnimReferenceHeight) {
            g_enemyAnimReferenceHeight = height;
        }
    };

    includeSize(g_enemyIdleGif);
    includeSize(g_enemyWalkGif);
    includeSize(g_enemyJumpGif);
    includeSize(g_enemySpecialAttackGif);
}

void LoadImageAsset(ImageAsset& asset, const TCHAR* path, int width = 0, int height = 0) {
    RenderUtils::LoadImageFlexible(asset.image, path, width, height);
    asset.hasAlpha = RenderUtils::HasImage(asset.image) && RenderUtils::HasMeaningfulAlpha(asset.image);
}

void ResetImageAsset(ImageAsset& asset) {
    asset.image.Resize(0, 0);
    asset.hasAlpha = false;
}

void BuildGrayWhiteImage(const IMAGE& source, bool sourceHasAlpha, IMAGE& target) {
    const int width = source.getwidth();
    const int height = source.getheight();
    target.Resize(0, 0);
    if (width <= 0 || height <= 0) {
        return;
    }

    target.Resize(width, height);
    DWORD* src = GetImageBuffer(const_cast<IMAGE*>(&source));
    DWORD* dst = GetImageBuffer(&target);
    if (src == nullptr || dst == nullptr) {
        target.Resize(0, 0);
        return;
    }

    const int pixelCount = width * height;
    for (int i = 0; i < pixelCount; ++i) {
        const DWORD pixel = src[i];
        const int alpha = static_cast<int>((pixel >> 24) & 0xFF);
        if (sourceHasAlpha && alpha <= 0) {
            dst[i] = 0;
            continue;
        }

        if (!sourceHasAlpha && (pixel & 0x00FFFFFF) == 0) {
            dst[i] = pixel;
            continue;
        }

        int r = static_cast<int>((pixel >> 16) & 0xFF);
        int g = static_cast<int>((pixel >> 8) & 0xFF);
        int b = static_cast<int>(pixel & 0xFF);
        if (sourceHasAlpha && alpha < 255) {
            r = (r * 255 + alpha / 2) / alpha;
            g = (g * 255 + alpha / 2) / alpha;
            b = (b * 255 + alpha / 2) / alpha;
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
        }

        int gray = (r * 30 + g * 59 + b * 11) / 100;
        gray = (gray + 230) / 2;
        if (sourceHasAlpha && alpha < 255) {
            gray = (gray * alpha + 127) / 255;
        }

        dst[i] = (pixel & 0xFF000000) |
            (static_cast<DWORD>(gray) << 16) |
            (static_cast<DWORD>(gray) << 8) |
            static_cast<DWORD>(gray);
    }
}

}  

void LoadAllCharacterGifs() {
    for (int i = 0; i < kCharacterCount; ++i) {
        RenderUtils::LoadAnimatedGif(g_idleGifs[i], AssetPaths::GetCharacterIdleGifPath(i));
        RenderUtils::LoadAnimatedGif(g_walkGifs[i], AssetPaths::GetCharacterWalkGifPath(i));
    }
    RenderUtils::LoadAnimatedGif(g_apolloSlashGif, AssetPaths::GetApolloSlashGifPath());
    LoadImageAsset(g_sunUltimateSwordQiAsset, AssetPaths::GetSunUltimateSwordQiPath());
    LoadImageAsset(g_sunCriticalHaloAsset, AssetPaths::GetSunCriticalHaloPath());
    LoadImageAsset(g_moonRainArrowAsset, AssetPaths::GetMoonRainArrowPath());
    LoadImageAsset(g_moonRainAimCircleAsset, AssetPaths::GetMoonRainAimCirclePath());
    LoadImageAsset(g_moonUltimateFireballAsset, AssetPaths::GetMoonUltimateFireballPath());
    LoadImageAsset(g_moonUltimateFireballAuraAsset, AssetPaths::GetMoonUltimateFireballAuraPath());
    LoadImageAsset(g_moonUltimateWaveAsset, AssetPaths::GetMoonUltimateWavePath());
    LoadImageAsset(g_moonMagicCageAsset, AssetPaths::GetMoonMagicCagePath());
    LoadImageAsset(g_sunUltimateShieldAsset, AssetPaths::GetSunUltimateShieldPath());
    LoadImageAsset(g_loveUltimateRecoverCircleAsset, AssetPaths::GetLoveUltimateRecoverCirclePath());
    LoadImageAsset(g_loveUltimateBulletAsset, AssetPaths::GetLoveUltimateBulletPath());
    LoadImageAsset(g_lovePurifyCircleAsset, AssetPaths::GetLovePurifyCirclePath());
    for (int i = 0; i < kCharacterCount; ++i) {
        LoadImageAsset(g_ultimateCooldownIconAssets[i], AssetPaths::GetUltimateCooldownIconPath(i));
        BuildGrayWhiteImage(
            g_ultimateCooldownIconAssets[i].image,
            g_ultimateCooldownIconAssets[i].hasAlpha,
            g_ultimateCooldownGrayIconAssets[i].image);
        g_ultimateCooldownGrayIconAssets[i].hasAlpha =
            RenderUtils::HasImage(g_ultimateCooldownGrayIconAssets[i].image) &&
            RenderUtils::HasMeaningfulAlpha(g_ultimateCooldownGrayIconAssets[i].image);
    }
}

void FreeAllCharacterGifs() {
    for (int i = 0; i < kCharacterCount; ++i) {
        g_idleGifs[i].Reset();
        g_walkGifs[i].Reset();
    }
    g_apolloSlashGif.Reset();
    g_apolloSlashState = {};
    g_apolloSlashNextAvailableTick = 0;
    ResetImageAsset(g_sunUltimateSwordQiAsset);
    ResetImageAsset(g_sunCriticalHaloAsset);
    g_sunSwordQiProjectiles.clear();
    g_sunCriticalHitCount = 0;
    ResetImageAsset(g_moonRainArrowAsset);
    ResetImageAsset(g_moonRainAimCircleAsset);
    ResetImageAsset(g_moonUltimateFireballAsset);
    ResetImageAsset(g_moonUltimateFireballAuraAsset);
    ResetImageAsset(g_moonUltimateWaveAsset);
    ResetImageAsset(g_moonMagicCageAsset);
    ResetImageAsset(g_sunUltimateShieldAsset);
    ResetImageAsset(g_loveUltimateRecoverCircleAsset);
    ResetImageAsset(g_loveUltimateBulletAsset);
    ResetImageAsset(g_lovePurifyCircleAsset);
    for (int i = 0; i < kCharacterCount; ++i) {
        ResetImageAsset(g_ultimateCooldownIconAssets[i]);
        ResetImageAsset(g_ultimateCooldownGrayIconAssets[i]);
    }
    g_moonRainChargeState = {};
    g_moonRainCastState = {};
    g_moonUltimateState = {};
    g_moonUltimateFireballs.clear();
    g_moonUltimateFireballNextCastTick = 0;
    g_sunUltimateState = {};
    g_loveUltimateState = {};
    g_lovePurifyState = {};
}

void LoadAllCharacterDeathImages() {
    for (int i = 0; i < kCharacterCount; ++i) {
        LoadImageAsset(g_playerDeathAssets[i], AssetPaths::GetCharacterDeathImagePath(i));
    }
}

void FreeAllCharacterDeathImages() {
    for (int i = 0; i < kCharacterCount; ++i) {
        ResetImageAsset(g_playerDeathAssets[i]);
    }
}

void LoadEnemyAssets() {
    const GameData::EnemyDefinition& enemyDef = EnemyDef();

    RenderUtils::LoadAnimatedGif(g_enemyIdleGif, enemyDef.idleGifPath);
    RenderUtils::LoadAnimatedGif(g_enemyWalkGif, enemyDef.walkGifPath);
    RenderUtils::LoadAnimatedGif(g_enemyBreakGif, AssetPaths::GetEliteBreakStatePath());
    g_enemySpecialAttackGif.Reset();
    g_enemyJumpGif.Reset();
    if (enemyDef.specialAttackGifPath != nullptr) {
        RenderUtils::LoadAnimatedGif(g_enemySpecialAttackGif, enemyDef.specialAttackGifPath);
    }
    if (enemyDef.kind == GameData::EnemyKind::SnowApeKing) {
        RenderUtils::LoadAnimatedGif(g_enemyJumpGif, AssetPaths::GetSnowApeKingEnemyJumpPath());
    }

    LoadImageAsset(g_enemyDeathAsset, enemyDef.deathImagePath);
    LoadImageAsset(g_enemyBulletAsset, enemyDef.bulletImagePath);
    LoadImageAsset(g_enemyIceSpikeAsset, AssetPaths::GetSnowApeKingIceSpikePath());

    RefreshEnemyAnimationReferenceSize();
}

void FreeEnemyAssets() {
    g_enemyIdleGif.Reset();
    g_enemyWalkGif.Reset();
    g_enemyBreakGif.Reset();
    g_enemySpecialAttackGif.Reset();
    g_enemyJumpGif.Reset();
    ResetImageAsset(g_enemyDeathAsset);
    ResetImageAsset(g_enemyBulletAsset);
    ResetImageAsset(g_enemyIceSpikeAsset);
    g_enemyAnimReferenceWidth = 0;
    g_enemyAnimReferenceHeight = 0;
}

void LoadParticleImage() {
    LoadImageAsset(g_particleAsset, AssetPaths::GetLoveParticlePath());
}

void LoadPlayerHpIconImage() {
    LoadImageAsset(g_playerHpIconAsset, AssetPaths::GetPlayerHpIconPath());
}

void LoadPlayerArmorIconImage() {
    LoadImageAsset(g_playerArmorIconAsset, AssetPaths::GetPlayerArmorIconPath());
}

void LoadPlayerEnergyIconImage() {
    LoadImageAsset(g_playerEnergyIconAsset, AssetPaths::GetPlayerEnergyIconPath());
}

void LoadEnergyDropImage() {
    LoadImageAsset(g_energyDropAsset, AssetPaths::GetEnergyDropIconPath());
    LoadImageAsset(g_recoverPotionDropAsset, AssetPaths::GetRecoverPotionDropIconPath());
    LoadImageAsset(g_energyPotionDropAsset, AssetPaths::GetEnergyPotionDropIconPath());
    LoadImageAsset(g_lifePotionDropAsset, AssetPaths::GetLifePotionDropIconPath());
}

}  
