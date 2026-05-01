#include "level_map_internal.h"

namespace LevelMapInternal {
namespace {

void StopPlayerDash(ULONGLONG now) {
    if (!g_isDashing) {
        return;
    }

    g_isDashing = false;
    g_dashCooldownEndTick = now + kPlayerDashCooldownMs;
}

bool CanStartPlayerDash(ULONGLONG now) {
    return now >= g_dashCooldownEndTick && g_playerEnergy > 0.0f;
}

void UpdatePlayerDashAfterimages(ULONGLONG now) {
    size_t writeIndex = 0;
    for (size_t i = 0; i < g_playerDashAfterimages.size(); ++i) {
        const ULONGLONG age = (now >= g_playerDashAfterimages[i].createdTick)
            ? (now - g_playerDashAfterimages[i].createdTick)
            : 0ULL;
        if (age <= kPlayerDashAfterimageLifetimeMs) {
            if (writeIndex != i) {
                g_playerDashAfterimages[writeIndex] = g_playerDashAfterimages[i];
            }
            ++writeIndex;
        }
    }
    g_playerDashAfterimages.resize(writeIndex);
}

void SpawnPlayerDashAfterimage(ULONGLONG now) {
    if (!g_isDashing || !g_isMoving) {
        return;
    }

    if (g_lastAfterimageSpawnTick != 0 && now - g_lastAfterimageSpawnTick < kPlayerDashAfterimageSpawnIntervalMs) {
        return;
    }

    DashAfterimage image;
    image.x = g_playerX;
    image.y = g_playerY;
    image.faceRight = g_faceRight;
    image.moving = g_isMoving;
    image.createdTick = now;
    PushCapped(g_playerDashAfterimages, image, kDashAfterimageReserveCount);
    g_lastAfterimageSpawnTick = now;
}

void UpdatePlayerArmorRegen(ULONGLONG now) {
    if (g_playerIsDead || g_playerMaxArmor <= 0 || g_playerArmor >= g_playerMaxArmor) {
        return;
    }

    const ULONGLONG regenStartTick = g_playerLastDamageTick + kPlayerArmorRegenDelayMs;
    if (now < regenStartTick) {
        return;
    }

    if (g_playerArmorRegenTick < regenStartTick) {
        g_playerArmorRegenTick = regenStartTick;
    }

    while (g_playerArmor < g_playerMaxArmor &&
        now >= g_playerArmorRegenTick + kPlayerArmorRegenIntervalMs) {
        g_playerArmor += kPlayerArmorRegenPerTick;
        if (g_playerArmor > g_playerMaxArmor) {
            g_playerArmor = g_playerMaxArmor;
        }
        g_playerArmorRegenTick += kPlayerArmorRegenIntervalMs;
    }
}

bool IsLovePurifyActive(ULONGLONG now) {
    return g_currentCharacterIndex == AssetPaths::CHARACTER_LOVE &&
        g_lovePurifyState.active &&
        now < g_lovePurifyState.endTick;
}

void UpdateLoveUltimateState(ULONGLONG now) {
    if (!g_loveUltimateState.active) {
        return;
    }

    while (g_loveUltimateState.nextHealTick != 0 &&
        now >= g_loveUltimateState.nextHealTick &&
        g_loveUltimateState.nextHealTick <= g_loveUltimateState.endTick) {
        if (g_playerHp < g_playerMaxHp) {
            g_playerHp += kLoveUltimateHealAmount;
            if (g_playerHp > g_playerMaxHp) {
                g_playerHp = g_playerMaxHp;
            }
        }
        g_loveUltimateState.nextHealTick += kLoveUltimateHealIntervalMs;
    }

    if (now >= g_loveUltimateState.endTick) {
        g_loveUltimateState = {};
    }
}

void UpdateLovePurifyState(ULONGLONG now) {
    if (g_lovePurifyState.active && now >= g_lovePurifyState.endTick) {
        g_lovePurifyState = {};
    }
}

void TriggerMoonUltimate(ULONGLONG now) {
    g_moonUltimateState.active = true;
    g_moonUltimateState.startTick = now;
    g_moonUltimateState.endTick = now + kMoonUltimateVisualDurationMs;
    g_moonRainChargeState = {};
    g_moonRainCastState = {};
    g_moonUltimateFireballs.clear();
    g_moonUltimateFireballNextCastTick = 0;
}

void TriggerSunUltimate(ULONGLONG now) {
    g_sunUltimateState.active = true;
    g_sunUltimateState.startTick = now;
    g_sunUltimateState.endTick = now + kMoonUltimateVisualDurationMs;
    
}

void TriggerLoveUltimate(ULONGLONG now) {
    g_loveUltimateState.active = true;
    g_loveUltimateState.startTick = now;
    g_loveUltimateState.endTick = now + kMoonUltimateVisualDurationMs;
    g_loveUltimateState.nextHealTick = now + kLoveUltimateHealIntervalMs;
}

void TryTriggerCharacterUltimate(ULONGLONG now) {
    if (g_currentCharacterIndex < 0 || g_currentCharacterIndex >= kCharacterCount) {
        return;
    }
    if (now < g_characterUltimateNextCastTick[g_currentCharacterIndex]) {
        return;
    }

    switch (g_currentCharacterIndex) {
    case AssetPaths::CHARACTER_MOON:
        TriggerMoonUltimate(now);
        break;
    case AssetPaths::CHARACTER_SUN:
        TriggerSunUltimate(now);
        break;
    case AssetPaths::CHARACTER_LOVE:
        TriggerLoveUltimate(now);
        break;
    default:
        return;
    }

    g_characterUltimateNextCastTick[g_currentCharacterIndex] =
        now + kMoonUltimateVisualDurationMs + kCharacterUltimateCooldownMs;
}

}  

void UpdatePlayerByKeyboard(ULONGLONG now) {
    if (g_lastPlayerUpdateTick == 0) {
        g_lastPlayerUpdateTick = now;
    }

    ULONGLONG deltaMs = (now >= g_lastPlayerUpdateTick) ? (now - g_lastPlayerUpdateTick) : 0ULL;
    if (deltaMs > 100ULL) {
        deltaMs = 100ULL;
    }
    g_lastPlayerUpdateTick = now;
    const float deltaSeconds = static_cast<float>(deltaMs) / 1000.0f;
    UpdatePlayerDashAfterimages(now);
    UpdatePlayerArmorRegen(now);
    UpdateLoveUltimateState(now);
    UpdateLovePurifyState(now);

    const bool ultimateKeyPressed =
        ((GetAsyncKeyState('Q') & 0x8000) != 0) ||
        ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0);
    const bool ultimateReady =
        g_currentCharacterIndex >= 0 &&
        g_currentCharacterIndex < kCharacterCount &&
        now >= g_characterUltimateNextCastTick[g_currentCharacterIndex];
    if (ultimateKeyPressed && (!g_ultimateShiftPressedLastFrame || ultimateReady)) {
        TryTriggerCharacterUltimate(now);
    }
    g_ultimateShiftPressedLastFrame = ultimateKeyPressed;

    int dx = 0;
    int dy = 0;

    if ((GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState(VK_LEFT) & 0x8000)) dx -= kPlayerSpeed;
    if ((GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000)) dx += kPlayerSpeed;
    if ((GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState(VK_UP) & 0x8000)) dy -= kPlayerSpeed;
    if ((GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState(VK_DOWN) & 0x8000)) dy += kPlayerSpeed;

    g_isMoving = (dx != 0 || dy != 0);
    if (dx < 0) g_faceRight = false;
    if (dx > 0) g_faceRight = true;

    const bool wantsDash = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    if (g_isDashing) {
        const ULONGLONG dashElapsed = now - g_dashStartTick;
        if (!g_isMoving || !wantsDash || dashElapsed >= kPlayerDashMaxDurationMs || g_playerEnergy <= 0.0f) {
            StopPlayerDash(now);
        }
    }
    else if (wantsDash && g_isMoving && CanStartPlayerDash(now)) {
        g_isDashing = true;
        g_dashStartTick = now;
    }

    if (g_isDashing) {
        g_playerEnergy -= kPlayerDashEnergyCostPerSecond * deltaSeconds;
        if (g_playerEnergy <= 0.0f) {
            g_playerEnergy = 0.0f;
            StopPlayerDash(now);
        }
    }

    if (g_isMoving) {
        if (g_isDashing) {
            dx *= kPlayerDashSpeedMultiplier;
            dy *= kPlayerDashSpeedMultiplier;
        }
        if (IsLovePurifyActive(now)) {
            dx = dx * kLovePurifyMoveNumerator / kLovePurifyMoveDenominator;
            dy = dy * kLovePurifyMoveNumerator / kLovePurifyMoveDenominator;
        }
        const int previousX = g_playerX;
        const int previousY = g_playerY;
        g_moveDirX = dx;
        g_moveDirY = dy;
        g_playerX += dx;
        g_playerY += dy;
        ClampPlayerToMapBounds();
        ResolvePlayerPositionAgainstObstacles(previousX, previousY);
    }

    SpawnPlayerDashAfterimage(now);
}

}  
