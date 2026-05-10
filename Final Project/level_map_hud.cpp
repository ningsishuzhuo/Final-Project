#include "level_map_internal.h"
#include "level_map_hud_internal.h"
#include "test_toggles.h"

namespace LevelMapInternal {
namespace {

enum class HudFallbackIcon {
    Circle,
    Square
};

void DrawResultOverlay(ULONGLONG now, ULONGLONG startTick, const TCHAR* title) {
    constexpr ULONGLONG kGameOverFadeInMs = 700ULL;
    const ULONGLONG elapsed =
        (startTick > 0 && now >= startTick) ? (now - startTick) : 0ULL;
    float progress = static_cast<float>(elapsed) / static_cast<float>(kGameOverFadeInMs);
    if (progress < 0.0f) {
        progress = 0.0f;
    }
    if (progress > 1.0f) {
        progress = 1.0f;
    }

    constexpr int kTargetTextR = 220;
    constexpr int kTargetTextG = 226;
    constexpr int kTargetTextB = 236;
    const int r = static_cast<int>(static_cast<float>(kTargetTextR) * progress);
    const int g = static_cast<int>(static_cast<float>(kTargetTextG) * progress);
    const int b = static_cast<int>(static_cast<float>(kTargetTextB) * progress);
    const COLORREF revealColor = RGB(r, g, b);

    setbkmode(TRANSPARENT);

    settextstyle(104, 0, _T("Type"), 0, 0, FW_BLACK, false, false, false);
    settextcolor(revealColor);
    RECT gameOverRect = { 0, GAME_WINDOW_HEIGHT / 2 - 130, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT / 2 + 30 };
    drawtext(title, &gameOverRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    if (elapsed >= kResultReturnDelayMs) {
        settextstyle(24, 0, _T("宋体"));
        settextcolor(revealColor);
        RECT hintRect = { 0, GAME_WINDOW_HEIGHT - 72, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT - 24 };
        drawtext(_T("按任意键返回"), &hintRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

void DrawHudIcon(
    const IMAGE& image,
    bool hasAlpha,
    int x,
    int y,
    int size,
    COLORREF fallbackColor,
    HudFallbackIcon fallbackIcon) {
    if (RenderUtils::HasImage(image)) {
        RenderUtils::DrawImageAuto(image, hasAlpha, x, y, size, size);
        return;
    }

    setfillcolor(fallbackColor);
    if (fallbackIcon == HudFallbackIcon::Circle) {
        solidcircle(x + size / 2, y + size / 2, size / 2);
        return;
    }

    solidrectangle(x, y, x + size, y + size);
}

void DrawHudDebugInfo(ULONGLONG now) {
    TCHAR info[512] = { 0 };
    const ULONGLONG dashCooldownMs =
        (!g_isDashing && g_dashCooldownEndTick > now) ? (g_dashCooldownEndTick - now) : 0ULL;
    const ULONGLONG dashElapsedMs = (now >= g_dashStartTick) ? (now - g_dashStartTick) : 0ULL;
    const ULONGLONG dashRemainingMs =
        g_isDashing ? (kPlayerDashMaxDurationMs > dashElapsedMs ? (kPlayerDashMaxDurationMs - dashElapsedMs) : 0ULL) : 0ULL;
    const GameData::EnemyDefinition& enemyDef = EnemyDef();
    int normalRemain = 0;
    int eliteRemain = 0;
    GetIcefieldRemainingEnemyCounts(normalRemain, eliteRemain);
    const TCHAR* phaseText = IsIcefieldNormalBattleActive() ? _T("普通战") : _T("Boss战");
    _stprintf_s(
        info,
        _countof(info),
        _T("角色:%d 能量:%d 阶段:%s 剩余(普/精):%d/%d 敌人:%s(%s) 模式:%d HP:%d/%d 敌弹:%d 震荡波:%d 破防:%s Target:(%d,%d) 冲刺剩余:%llums 冷却:%llums"),
        g_currentCharacterIndex + 1,
        static_cast<int>(g_playerEnergy),
        phaseText,
        normalRemain,
        eliteRemain,
        enemyDef.displayName,
        g_enemy.alive ? _T("存活") : _T("死亡"),
        static_cast<int>(enemyDef.attackMode),
        g_enemy.hp > 0 ? g_enemy.hp : 0,
        g_enemy.maxHp,
        static_cast<int>(g_enemyProjectiles.size()),
        static_cast<int>(g_enemyShockwaves.size()),
        g_enemy.breakStateActive ? _T("已触发") : _T("未触发"),
        g_enemy.roamTargetX,
        g_enemy.roamTargetY,
        dashRemainingMs,
        dashCooldownMs);

    RECT descRect = { 20, 70, GAME_WINDOW_WIDTH - 20, 100 };
    drawtext(info, &descRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}
}

void DrawGameOverOverlay(ULONGLONG now) {
    DrawResultOverlay(now, g_playerDeathStartTick, _T("GAME OVER"));
}

void DrawVictoryOverlay(ULONGLONG now) {
    DrawResultOverlay(now, g_gameVictoryStartTick, _T("VICTORY"));
}

void DrawHud(ULONGLONG now) {
    const int hpIconX = kPlayerHpHudLeft;
    const int hpIconY = kPlayerHpHudTop;
    const int statusRowHeight = kPlayerHpIconDrawSize + 6;
    const int valueLeftX = hpIconX + kPlayerHpIconDrawSize + 10;
    const int valueRightX = hpIconX + kPlayerHpIconDrawSize + 170;
    DrawHudIcon(
        g_playerHpIconAsset.image,
        g_playerHpIconAsset.hasAlpha,
        hpIconX,
        hpIconY,
        kPlayerHpIconDrawSize,
        RGB(220, 42, 52),
        HudFallbackIcon::Circle);

    TCHAR hpText[64] = { 0 };
    _stprintf_s(hpText, _countof(hpText), _T("%d/%d"), g_playerHp, g_playerMaxHp);
    settextstyle(24, 0, _T("黑体"));
    settextcolor(RGB(255, 248, 248));
    RECT hpRect = {
        valueLeftX,
        hpIconY,
        valueRightX,
        hpIconY + kPlayerHpIconDrawSize
    };
    drawtext(hpText, &hpRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    const int armorY = hpIconY + statusRowHeight;
    const int energyY = armorY + statusRowHeight;

    DrawHudIcon(
        g_playerArmorIconAsset.image,
        g_playerArmorIconAsset.hasAlpha,
        hpIconX,
        armorY,
        kPlayerHpIconDrawSize,
        RGB(190, 196, 206),
        HudFallbackIcon::Square);
    DrawHudIcon(
        g_playerEnergyIconAsset.image,
        g_playerEnergyIconAsset.hasAlpha,
        hpIconX,
        energyY,
        kPlayerHpIconDrawSize,
        RGB(98, 210, 255),
        HudFallbackIcon::Circle);

    TCHAR armorText[64] = { 0 };
    TCHAR energyText[64] = { 0 };
    _stprintf_s(armorText, _countof(armorText), _T("%d/%d"), g_playerArmor, g_playerMaxArmor);
    _stprintf_s(
        energyText,
        _countof(energyText),
        _T("%d/%d"),
        static_cast<int>(g_playerEnergy),
        g_playerMaxEnergy);

    settextstyle(24, 0, _T("黑体"));
    settextcolor(RGB(248, 252, 255));
    RECT armorRect = { valueLeftX, armorY, valueRightX, armorY + kPlayerHpIconDrawSize };
    RECT energyRect = { valueLeftX, energyY, valueRightX, energyY + kPlayerHpIconDrawSize };
    drawtext(armorText, &armorRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    drawtext(energyText, &energyRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawUltimateCooldownHud(now);
    DrawMinimap();

    if (TestToggles::Get().showAuxiliaryHudText) {
        settextstyle(20, 0, _T("宋体"));
        settextcolor(RGB(220, 226, 236));
        RECT titleRect = { 20, 44, GAME_WINDOW_WIDTH - 20, 74 };
        drawtext(_T("第一关：冰原"), &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        DrawHudDebugInfo(now);
    }
}
}
