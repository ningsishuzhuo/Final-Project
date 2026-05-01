#include "level_map_internal.h"
#include "level_map_hud_internal.h"

namespace LevelMapInternal {
namespace {
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

}  

void DrawGameOverOverlay(ULONGLONG now) {
    DrawResultOverlay(now, g_playerDeathStartTick, _T("GAME OVER"));
}

void DrawVictoryOverlay(ULONGLONG now) {
    DrawResultOverlay(now, g_gameVictoryStartTick, _T("VICTORY"));
}

}  
