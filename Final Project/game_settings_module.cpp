#include "game_settings_module.h"

#include "menu_music.h"
#include "menu_render_utils.h"
#include "render_utils.h"

#include <graphics.h>
#include <cstdio>
#include <tchar.h>

namespace {

constexpr int kSettingsPageCount = 2;
constexpr const TCHAR* kSettingsPageTitles[kSettingsPageCount] = {
    _T("操作"),
    _T("音量")
};

enum SettingsActionButton {
    SETTINGS_BTN_BACK = 0,
    SETTINGS_BTN_PREV = 1,
    SETTINGS_BTN_NEXT = 2,
    SETTINGS_BTN_COUNT = 3
};

constexpr const TCHAR* kSettingsActionLabels[SETTINGS_BTN_COUNT] = {
    _T("返回"),
    _T("上一项"),
    _T("下一项")
};

constexpr int kTitleFontSize = 74;    
constexpr int kActionFontSize = 32;   
constexpr int kActionButtonPadding = 20;
constexpr int kVolumeFontSize = 28;   
constexpr int kVolumeButtonPadding = 14;
constexpr COLORREF kSettingsBackgroundColor = RGB(7, 11, 22);  
constexpr int kSettingsContentTop = 112;
constexpr int kOperationLineHeight = 56;
constexpr int kOperationLineGap = 12;
constexpr int kOperationLineCount = 6;
constexpr int kVolumeLabelHeight = 56;
constexpr int kVolumeLineStep = kOperationLineHeight + kOperationLineGap;
constexpr int kVolumeGameControlY = 382;

enum VolumeButtonId {
    VOL_BTN_BG_MINUS = 0,
    VOL_BTN_BG_PLUS = 1,
    VOL_BTN_GAME_MINUS = 2,
    VOL_BTN_GAME_PLUS = 3,
    VOL_BTN_COUNT = 4
};

constexpr const TCHAR* kVolumeRowLabels[2] = {
    _T("背景音量"),
    _T("游戏音量")
};

constexpr const TCHAR* kVolumeButtonLabels[VOL_BTN_COUNT] = {
    _T("-"),
    _T("+"),
    _T("-"),
    _T("+")
};

RECT g_actionRects[SETTINGS_BTN_COUNT];
bool g_actionHovered[SETTINGS_BTN_COUNT] = { false, false, false };
RECT g_volumeButtonRects[VOL_BTN_COUNT];
bool g_volumeButtonHovered[VOL_BTN_COUNT] = { false, false, false };
int g_currentPage = 0;

void InitializeActionRects() {
    g_actionRects[SETTINGS_BTN_BACK] = { 28, 24, 28 + 140, 24 + 50 };
    g_actionRects[SETTINGS_BTN_PREV] = { 30, GAME_WINDOW_HEIGHT - 72, 30 + 190, GAME_WINDOW_HEIGHT - 22 };
    g_actionRects[SETTINGS_BTN_NEXT] = { GAME_WINDOW_WIDTH - 30 - 190, GAME_WINDOW_HEIGHT - 72, GAME_WINDOW_WIDTH - 30, GAME_WINDOW_HEIGHT - 22 };
    for (int i = 0; i < SETTINGS_BTN_COUNT; ++i) {
        g_actionHovered[i] = false;
    }
}

void InitializeVolumeButtonRects() {
    const int centerX = GAME_WINDOW_WIDTH / 2;
    const int buttonW = 52;
    const int buttonH = 52;
    const int row1Y = kVolumeGameControlY - kVolumeLineStep * 2;
    const int row2Y = kVolumeGameControlY;
    const int minusX = centerX - 130;
    const int plusX = centerX + 78;

    g_volumeButtonRects[VOL_BTN_BG_MINUS] = { minusX, row1Y, minusX + buttonW, row1Y + buttonH };
    g_volumeButtonRects[VOL_BTN_BG_PLUS] = { plusX, row1Y, plusX + buttonW, row1Y + buttonH };
    g_volumeButtonRects[VOL_BTN_GAME_MINUS] = { minusX, row2Y, minusX + buttonW, row2Y + buttonH };
    g_volumeButtonRects[VOL_BTN_GAME_PLUS] = { plusX, row2Y, plusX + buttonW, row2Y + buttonH };

    for (int i = 0; i < VOL_BTN_COUNT; ++i) {
        g_volumeButtonHovered[i] = false;
    }
}

void DrawTitle() {
    RECT titleRect = { 0, 18, GAME_WINDOW_WIDTH, 120 };
    MenuRenderUtils::DrawShadowedText(
        kSettingsPageTitles[g_currentPage],
        titleRect,
        kTitleFontSize,
        RGB(236, 244, 255),
        RGB(36, 54, 90),
        2);
}

void DrawBackground() {
    setbkcolor(kSettingsBackgroundColor);
    cleardevice();
}

void DrawActionButtons() {
    for (int i = 0; i < SETTINGS_BTN_COUNT; ++i) {
        const RECT& rect = g_actionRects[i];
        const bool hovered = g_actionHovered[i];
        const COLORREF mainColor = hovered ? RGB(255, 255, 255) : RGB(208, 222, 244);
        const COLORREF shadowColor = hovered ? RGB(98, 128, 182) : RGB(36, 54, 90);

        MenuRenderUtils::DrawShadowedText(kSettingsActionLabels[i], rect, kActionFontSize, mainColor, shadowColor);
    }
}

void DrawOperationPage() {
    setbkmode(TRANSPARENT);
    settextstyle(28, 0, _T("黑体"));
    settextcolor(RGB(220, 232, 248));

    const TCHAR* lines[6] = {
        _T("W A S D : 移动"),
        _T("鼠标左键 : 攻击"),
        _T("Q / Shift : 释放角色大招"),
        _T("Tab : 显示地图"),
        _T("空格 : 冲刺 / 加速"),
        _T("ESC : 返回主菜单")
    };
    for (int i = 0; i < kOperationLineCount; ++i) {
        const int top = kSettingsContentTop + i * (kOperationLineHeight + kOperationLineGap);
        RECT lineRect = { 90, top, GAME_WINDOW_WIDTH - 90, top + kOperationLineHeight };
        drawtext(lines[i], &lineRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

void DrawVolumeButton(VolumeButtonId buttonId, const TCHAR* label) {
    const RECT& rect = g_volumeButtonRects[buttonId];
    const bool hovered = g_volumeButtonHovered[buttonId];
    const COLORREF mainColor = hovered ? RGB(255, 255, 255) : RGB(208, 222, 244);
    const COLORREF shadowColor = hovered ? RGB(98, 128, 182) : RGB(36, 54, 90);

    MenuRenderUtils::DrawShadowedText(label, rect, kVolumeFontSize, mainColor, shadowColor);
}

void DrawVolumePage() {
    setbkmode(TRANSPARENT);
    settextstyle(kVolumeFontSize, 0, _T("黑体"));
    settextcolor(RGB(220, 232, 248));

    const int centerX = GAME_WINDOW_WIDTH / 2;
    const int row1LabelY = kVolumeGameControlY - kVolumeLineStep * 3;
    const int row2LabelY = kVolumeGameControlY - kVolumeLineStep;
    RECT bgLabelRect = { centerX - 220, row1LabelY, centerX + 220, row1LabelY + kVolumeLabelHeight };
    RECT gameLabelRect = { centerX - 220, row2LabelY, centerX + 220, row2LabelY + kVolumeLabelHeight };
    drawtext(kVolumeRowLabels[0], &bgLabelRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    drawtext(kVolumeRowLabels[1], &gameLabelRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    TCHAR bgVolumeText[16] = { 0 };
    TCHAR gameVolumeText[16] = { 0 };
    _stprintf_s(bgVolumeText, _T("%d"), MenuMusic::GetBackgroundVolumeLevel());
    _stprintf_s(gameVolumeText, _T("%d"), MenuMusic::GetGameVolumeLevel());

    for (int i = 0; i < VOL_BTN_COUNT; ++i) {
        DrawVolumeButton(static_cast<VolumeButtonId>(i), kVolumeButtonLabels[i]);
    }

    RECT bgValueRect = { centerX - 34, g_volumeButtonRects[VOL_BTN_BG_MINUS].top, centerX + 34, g_volumeButtonRects[VOL_BTN_BG_MINUS].bottom };
    RECT gameValueRect = { centerX - 34, g_volumeButtonRects[VOL_BTN_GAME_MINUS].top, centerX + 34, g_volumeButtonRects[VOL_BTN_GAME_MINUS].bottom };
    settextcolor(RGB(236, 244, 255));
    drawtext(bgVolumeText, &bgValueRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    drawtext(gameVolumeText, &gameValueRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void UpdateHover(int x, int y) {
    for (int i = 0; i < SETTINGS_BTN_COUNT; ++i) {
        RECT hoverRect = g_actionRects[i];
        hoverRect.left -= kActionButtonPadding;
        hoverRect.right += kActionButtonPadding;
        hoverRect.top -= kActionButtonPadding;
        hoverRect.bottom += kActionButtonPadding;
        g_actionHovered[i] = RenderUtils::IsPointInRect(x, y, hoverRect);
    }

    for (int i = 0; i < VOL_BTN_COUNT; ++i) {
        RECT hoverRect = g_volumeButtonRects[i];
        hoverRect.left -= kVolumeButtonPadding;
        hoverRect.right += kVolumeButtonPadding;
        hoverRect.top -= kVolumeButtonPadding;
        hoverRect.bottom += kVolumeButtonPadding;
        g_volumeButtonHovered[i] = RenderUtils::IsPointInRect(x, y, hoverRect);
    }
}

}  

namespace GameSettingsModule {

void Initialize() {
    MenuRenderUtils::AcquireMenuFont();
    InitializeActionRects();
    InitializeVolumeButtonRects();
    g_currentPage = 0;
}

void Shutdown() {
    MenuRenderUtils::ReleaseMenuFont();
}

void EnterSettings() {
    g_currentPage = 0;
    for (int i = 0; i < SETTINGS_BTN_COUNT; ++i) {
        g_actionHovered[i] = false;
    }
    for (int i = 0; i < VOL_BTN_COUNT; ++i) {
        g_volumeButtonHovered[i] = false;
    }
}

void Draw() {
    DrawBackground();
    DrawTitle();
    if (g_currentPage == 0) {
        DrawOperationPage();
    } else {
        DrawVolumePage();
    }
    DrawActionButtons();
}

void HandleMouseMove(int x, int y) {
    UpdateHover(x, y);
}

void HandleMouseClick(int x, int y, GameState& currentState) {
    if (g_currentPage == 1) {
        if (RenderUtils::IsPointInRect(x, y, g_volumeButtonRects[VOL_BTN_BG_MINUS])) {
            MenuMusic::AdjustBackgroundVolume(-1);
            return;
        }
        if (RenderUtils::IsPointInRect(x, y, g_volumeButtonRects[VOL_BTN_BG_PLUS])) {
            MenuMusic::AdjustBackgroundVolume(1);
            return;
        }
        if (RenderUtils::IsPointInRect(x, y, g_volumeButtonRects[VOL_BTN_GAME_MINUS])) {
            MenuMusic::AdjustGameVolume(-1);
            return;
        }
        if (RenderUtils::IsPointInRect(x, y, g_volumeButtonRects[VOL_BTN_GAME_PLUS])) {
            MenuMusic::AdjustGameVolume(1);
            return;
        }
    }

    if (RenderUtils::IsPointInRect(x, y, g_actionRects[SETTINGS_BTN_BACK])) {
        currentState = MAIN_MENU;
        return;
    }
    if (RenderUtils::IsPointInRect(x, y, g_actionRects[SETTINGS_BTN_PREV])) {
        g_currentPage = (g_currentPage - 1 + kSettingsPageCount) % kSettingsPageCount;
        return;
    }
    if (RenderUtils::IsPointInRect(x, y, g_actionRects[SETTINGS_BTN_NEXT])) {
        g_currentPage = (g_currentPage + 1) % kSettingsPageCount;
        return;
    }
}

}  
