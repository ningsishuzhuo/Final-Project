#include "level_map_internal.h"
#include "level_map_hud_internal.h"
#include "test_toggles.h"

namespace LevelMapInternal {
namespace {
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
void DrawHud(ULONGLONG now) {
    const int hpIconX = kPlayerHpHudLeft;
    const int hpIconY = kPlayerHpHudTop;
    const int statusRowHeight = kPlayerHpIconDrawSize + 6;
    const int valueLeftX = hpIconX + kPlayerHpIconDrawSize + 10;
    const int valueRightX = hpIconX + kPlayerHpIconDrawSize + 170;
    if (RenderUtils::HasImage(g_playerHpIconImage)) {
        RenderUtils::DrawImageAuto(
            g_playerHpIconImage,
            g_playerHpIconHasAlpha,
            hpIconX,
            hpIconY,
            kPlayerHpIconDrawSize,
            kPlayerHpIconDrawSize);
    }
    else {
        setfillcolor(RGB(220, 42, 52));
        solidcircle(
            hpIconX + kPlayerHpIconDrawSize / 2,
            hpIconY + kPlayerHpIconDrawSize / 2,
            kPlayerHpIconDrawSize / 2);
    }

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

    if (RenderUtils::HasImage(g_playerArmorIconImage)) {
        RenderUtils::DrawImageAuto(
            g_playerArmorIconImage,
            g_playerArmorIconHasAlpha,
            hpIconX,
            armorY,
            kPlayerHpIconDrawSize,
            kPlayerHpIconDrawSize);
    }
    else {
        setfillcolor(RGB(190, 196, 206));
        solidrectangle(hpIconX, armorY, hpIconX + kPlayerHpIconDrawSize, armorY + kPlayerHpIconDrawSize);
    }

    if (RenderUtils::HasImage(g_playerEnergyIconImage)) {
        RenderUtils::DrawImageAuto(
            g_playerEnergyIconImage,
            g_playerEnergyIconHasAlpha,
            hpIconX,
            energyY,
            kPlayerHpIconDrawSize,
            kPlayerHpIconDrawSize);
    }
    else {
        setfillcolor(RGB(98, 210, 255));
        solidcircle(
            hpIconX + kPlayerHpIconDrawSize / 2,
            energyY + kPlayerHpIconDrawSize / 2,
            kPlayerHpIconDrawSize / 2);
    }

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
