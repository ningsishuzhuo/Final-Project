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
    RenderUtils::LoadImageFlexible(g_sunUltimateSwordQiImage, AssetPaths::GetSunUltimateSwordQiPath(), 0, 0);
    g_sunUltimateSwordQiHasAlpha =
        RenderUtils::HasImage(g_sunUltimateSwordQiImage) && RenderUtils::HasMeaningfulAlpha(g_sunUltimateSwordQiImage);
    RenderUtils::LoadImageFlexible(g_sunCriticalHaloImage, AssetPaths::GetSunCriticalHaloPath(), 0, 0);
    g_sunCriticalHaloHasAlpha =
        RenderUtils::HasImage(g_sunCriticalHaloImage) && RenderUtils::HasMeaningfulAlpha(g_sunCriticalHaloImage);
    RenderUtils::LoadImageFlexible(g_moonRainArrowImage, AssetPaths::GetMoonRainArrowPath(), 0, 0);
    g_moonRainArrowHasAlpha =
        RenderUtils::HasImage(g_moonRainArrowImage) && RenderUtils::HasMeaningfulAlpha(g_moonRainArrowImage);
    RenderUtils::LoadImageFlexible(g_moonRainAimCircleImage, AssetPaths::GetMoonRainAimCirclePath(), 0, 0);
    g_moonRainAimCircleHasAlpha =
        RenderUtils::HasImage(g_moonRainAimCircleImage) && RenderUtils::HasMeaningfulAlpha(g_moonRainAimCircleImage);
    RenderUtils::LoadImageFlexible(g_moonUltimateFireballImage, AssetPaths::GetMoonUltimateFireballPath(), 0, 0);
    g_moonUltimateFireballHasAlpha =
        RenderUtils::HasImage(g_moonUltimateFireballImage) && RenderUtils::HasMeaningfulAlpha(g_moonUltimateFireballImage);
    RenderUtils::LoadImageFlexible(g_moonUltimateFireballAuraImage, AssetPaths::GetMoonUltimateFireballAuraPath(), 0, 0);
    g_moonUltimateFireballAuraHasAlpha =
        RenderUtils::HasImage(g_moonUltimateFireballAuraImage) &&
        RenderUtils::HasMeaningfulAlpha(g_moonUltimateFireballAuraImage);
    RenderUtils::LoadImageFlexible(g_moonUltimateWaveImage, AssetPaths::GetMoonUltimateWavePath(), 0, 0);
    g_moonUltimateWaveHasAlpha =
        RenderUtils::HasImage(g_moonUltimateWaveImage) && RenderUtils::HasMeaningfulAlpha(g_moonUltimateWaveImage);
    RenderUtils::LoadImageFlexible(g_moonMagicCageImage, AssetPaths::GetMoonMagicCagePath(), 0, 0);
    g_moonMagicCageHasAlpha =
        RenderUtils::HasImage(g_moonMagicCageImage) && RenderUtils::HasMeaningfulAlpha(g_moonMagicCageImage);
    RenderUtils::LoadImageFlexible(g_sunUltimateShieldImage, AssetPaths::GetSunUltimateShieldPath(), 0, 0);
    g_sunUltimateShieldHasAlpha =
        RenderUtils::HasImage(g_sunUltimateShieldImage) && RenderUtils::HasMeaningfulAlpha(g_sunUltimateShieldImage);
    RenderUtils::LoadImageFlexible(g_loveUltimateRecoverCircleImage, AssetPaths::GetLoveUltimateRecoverCirclePath(), 0, 0);
    g_loveUltimateRecoverCircleHasAlpha =
        RenderUtils::HasImage(g_loveUltimateRecoverCircleImage) && RenderUtils::HasMeaningfulAlpha(g_loveUltimateRecoverCircleImage);
    RenderUtils::LoadImageFlexible(g_loveUltimateBulletImage, AssetPaths::GetLoveUltimateBulletPath(), 0, 0);
    g_loveUltimateBulletHasAlpha =
        RenderUtils::HasImage(g_loveUltimateBulletImage) && RenderUtils::HasMeaningfulAlpha(g_loveUltimateBulletImage);
    RenderUtils::LoadImageFlexible(g_lovePurifyCircleImage, AssetPaths::GetLovePurifyCirclePath(), 0, 0);
    g_lovePurifyCircleHasAlpha =
        RenderUtils::HasImage(g_lovePurifyCircleImage) && RenderUtils::HasMeaningfulAlpha(g_lovePurifyCircleImage);
    for (int i = 0; i < kCharacterCount; ++i) {
        RenderUtils::LoadImageFlexible(g_ultimateCooldownIconImages[i], AssetPaths::GetUltimateCooldownIconPath(i), 0, 0);
        g_ultimateCooldownIconHasAlpha[i] =
            RenderUtils::HasImage(g_ultimateCooldownIconImages[i]) &&
            RenderUtils::HasMeaningfulAlpha(g_ultimateCooldownIconImages[i]);
        BuildGrayWhiteImage(
            g_ultimateCooldownIconImages[i],
            g_ultimateCooldownIconHasAlpha[i],
            g_ultimateCooldownGrayIconImages[i]);
        g_ultimateCooldownGrayIconHasAlpha[i] =
            RenderUtils::HasImage(g_ultimateCooldownGrayIconImages[i]) &&
            RenderUtils::HasMeaningfulAlpha(g_ultimateCooldownGrayIconImages[i]);
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
    g_sunUltimateSwordQiImage.Resize(0, 0);
    g_sunUltimateSwordQiHasAlpha = false;
    g_sunCriticalHaloImage.Resize(0, 0);
    g_sunCriticalHaloHasAlpha = false;
    g_sunSwordQiProjectiles.clear();
    g_sunCriticalHitCount = 0;
    g_moonRainArrowImage.Resize(0, 0);
    g_moonRainArrowHasAlpha = false;
    g_moonRainAimCircleImage.Resize(0, 0);
    g_moonRainAimCircleHasAlpha = false;
    g_moonUltimateFireballImage.Resize(0, 0);
    g_moonUltimateFireballHasAlpha = false;
    g_moonUltimateFireballAuraImage.Resize(0, 0);
    g_moonUltimateFireballAuraHasAlpha = false;
    g_moonUltimateWaveImage.Resize(0, 0);
    g_moonUltimateWaveHasAlpha = false;
    g_moonMagicCageImage.Resize(0, 0);
    g_moonMagicCageHasAlpha = false;
    g_sunUltimateShieldImage.Resize(0, 0);
    g_sunUltimateShieldHasAlpha = false;
    g_loveUltimateRecoverCircleImage.Resize(0, 0);
    g_loveUltimateRecoverCircleHasAlpha = false;
    g_loveUltimateBulletImage.Resize(0, 0);
    g_loveUltimateBulletHasAlpha = false;
    g_lovePurifyCircleImage.Resize(0, 0);
    g_lovePurifyCircleHasAlpha = false;
    for (int i = 0; i < kCharacterCount; ++i) {
        g_ultimateCooldownIconImages[i].Resize(0, 0);
        g_ultimateCooldownIconHasAlpha[i] = false;
        g_ultimateCooldownGrayIconImages[i].Resize(0, 0);
        g_ultimateCooldownGrayIconHasAlpha[i] = false;
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
        RenderUtils::LoadImageFlexible(g_playerDeathImages[i], AssetPaths::GetCharacterDeathImagePath(i), 0, 0);
        g_playerDeathHasAlpha[i] =
            RenderUtils::HasImage(g_playerDeathImages[i]) && RenderUtils::HasMeaningfulAlpha(g_playerDeathImages[i]);
    }
}

void FreeAllCharacterDeathImages() {
    for (int i = 0; i < kCharacterCount; ++i) {
        g_playerDeathImages[i].Resize(0, 0);
        g_playerDeathHasAlpha[i] = false;
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

    RenderUtils::LoadImageFlexible(g_enemyDeathImage, enemyDef.deathImagePath, 0, 0);
    g_enemyDeathHasAlpha = RenderUtils::HasImage(g_enemyDeathImage) && RenderUtils::HasMeaningfulAlpha(g_enemyDeathImage);

    RenderUtils::LoadImageFlexible(g_enemyBulletImage, enemyDef.bulletImagePath, 0, 0);
    g_enemyBulletHasAlpha = RenderUtils::HasImage(g_enemyBulletImage) && RenderUtils::HasMeaningfulAlpha(g_enemyBulletImage);

    RenderUtils::LoadImageFlexible(g_enemyIceSpikeImage, AssetPaths::GetSnowApeKingIceSpikePath(), 0, 0);
    g_enemyIceSpikeHasAlpha = RenderUtils::HasImage(g_enemyIceSpikeImage) && RenderUtils::HasMeaningfulAlpha(g_enemyIceSpikeImage);

    RefreshEnemyAnimationReferenceSize();
}

void FreeEnemyAssets() {
    g_enemyIdleGif.Reset();
    g_enemyWalkGif.Reset();
    g_enemyBreakGif.Reset();
    g_enemySpecialAttackGif.Reset();
    g_enemyJumpGif.Reset();
    g_enemyDeathImage.Resize(0, 0);
    g_enemyBulletImage.Resize(0, 0);
    g_enemyIceSpikeImage.Resize(0, 0);
    g_enemyDeathHasAlpha = false;
    g_enemyBulletHasAlpha = false;
    g_enemyIceSpikeHasAlpha = false;
    g_enemyAnimReferenceWidth = 0;
    g_enemyAnimReferenceHeight = 0;
}

void LoadParticleImage() {
    RenderUtils::LoadImageFlexible(g_particleImage, AssetPaths::GetLoveParticlePath(), 0, 0);
    g_particleHasAlpha = RenderUtils::HasImage(g_particleImage) && RenderUtils::HasMeaningfulAlpha(g_particleImage);
}

void LoadPlayerHpIconImage() {
    RenderUtils::LoadImageFlexible(g_playerHpIconImage, AssetPaths::GetPlayerHpIconPath(), 0, 0);
    g_playerHpIconHasAlpha =
        RenderUtils::HasImage(g_playerHpIconImage) && RenderUtils::HasMeaningfulAlpha(g_playerHpIconImage);
}

void LoadPlayerArmorIconImage() {
    RenderUtils::LoadImageFlexible(g_playerArmorIconImage, AssetPaths::GetPlayerArmorIconPath(), 0, 0);
    g_playerArmorIconHasAlpha =
        RenderUtils::HasImage(g_playerArmorIconImage) && RenderUtils::HasMeaningfulAlpha(g_playerArmorIconImage);
}

void LoadPlayerEnergyIconImage() {
    RenderUtils::LoadImageFlexible(g_playerEnergyIconImage, AssetPaths::GetPlayerEnergyIconPath(), 0, 0);
    g_playerEnergyIconHasAlpha =
        RenderUtils::HasImage(g_playerEnergyIconImage) && RenderUtils::HasMeaningfulAlpha(g_playerEnergyIconImage);
}

void LoadEnergyDropImage() {
    RenderUtils::LoadImageFlexible(g_energyDropImage, AssetPaths::GetEnergyDropIconPath(), 0, 0);
    g_energyDropHasAlpha = RenderUtils::HasImage(g_energyDropImage) && RenderUtils::HasMeaningfulAlpha(g_energyDropImage);
    RenderUtils::LoadImageFlexible(g_recoverPotionDropImage, AssetPaths::GetRecoverPotionDropIconPath(), 0, 0);
    g_recoverPotionDropHasAlpha =
        RenderUtils::HasImage(g_recoverPotionDropImage) && RenderUtils::HasMeaningfulAlpha(g_recoverPotionDropImage);
    RenderUtils::LoadImageFlexible(g_energyPotionDropImage, AssetPaths::GetEnergyPotionDropIconPath(), 0, 0);
    g_energyPotionDropHasAlpha =
        RenderUtils::HasImage(g_energyPotionDropImage) && RenderUtils::HasMeaningfulAlpha(g_energyPotionDropImage);
    RenderUtils::LoadImageFlexible(g_lifePotionDropImage, AssetPaths::GetLifePotionDropIconPath(), 0, 0);
    g_lifePotionDropHasAlpha =
        RenderUtils::HasImage(g_lifePotionDropImage) && RenderUtils::HasMeaningfulAlpha(g_lifePotionDropImage);
}

}  
