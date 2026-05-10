#include "character_select_module.h"

#include <graphics.h>
#include <tchar.h>
#include "asset_paths.h"
#include "game_data.h"
#include "menu_render_utils.h"
#include "render_utils.h"
#ifndef WINVER
#define WINVER 0x0600
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>
#include <wingdi.h>
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "gdiplus.lib")

namespace {
constexpr int kCharacterCount = AssetPaths::CHARACTER_COUNT;
constexpr int kCharacterActionCount = 3;

constexpr const TCHAR* kCharacterActionLabels[kCharacterActionCount] = {
    _T("上一角色"),
    _T("下一角色"),
    _T("我已选好")
};

using RenderUtils::AnimatedGif;

struct ImageAsset {
    IMAGE image;
    bool hasAlpha = false;
};

ImageAsset g_characterPortraits[kCharacterCount];
ImageAsset g_characterMoons[kCharacterCount];
AnimatedGif g_characterGifs[kCharacterCount];
IMAGE g_cachedBackground;
bool g_backgroundCacheBuilt = false;

RECT g_characterActionRects[kCharacterActionCount];
bool g_characterActionHovered[kCharacterActionCount] = { false, false, false };

int g_selectedCharacterIndex = 0;

constexpr int kContentPadding = 24;
constexpr int kPortraitX = 40;
constexpr int kPortraitY = 58;
constexpr int kPortraitW = 400;
constexpr int kPortraitH = 500;

constexpr int kRightColumnX = 470;
constexpr int kRightColumnW = 296;
constexpr int kRightCenterX = kRightColumnX + kRightColumnW / 2;

constexpr int kNameX = 506;
constexpr int kNameY = 66;
constexpr int kNameW = 246;
constexpr int kNameH = 160;

constexpr int kRightLeftCenterX = 536;
constexpr int kRightRightCenterX = 700;

constexpr int kWeaponW = 82;
constexpr int kWeaponH = 46;
constexpr int kWeaponX = kRightLeftCenterX - kWeaponW / 2;
constexpr int kWeaponY = 238;

constexpr int kPersonW = 82;
constexpr int kPersonH = 46;
constexpr int kPersonX = kRightRightCenterX - kPersonW / 2;
constexpr int kPersonY = 238;

constexpr int kMoonW = 136;
constexpr int kMoonH = 136;
constexpr int kMoonX = kRightLeftCenterX - kMoonW / 2;
constexpr int kMoonY = 300;

constexpr int kGifW = 136;
constexpr int kGifH = 136;
constexpr int kGifX = kRightRightCenterX - kGifW / 2;
constexpr int kGifY = 300;
constexpr int kGifInnerOffsetY = -6;

constexpr int kNavButtonW = 142;
constexpr int kNavButtonH = 44;
constexpr int kConfirmButtonW = 188;
constexpr int kConfirmButtonH = 48;
constexpr int kButtonsRowY = 480;

void DrawTypeLabel(const TCHAR* text, const RECT& rect, int fontSize, COLORREF mainColor, COLORREF shadowColor) {
    MenuRenderUtils::DrawShadowedText(text, rect, fontSize, mainColor, shadowColor, 2);
}

void LoadImageAsset(ImageAsset& asset, const TCHAR* path, int width = 0, int height = 0) {
    RenderUtils::LoadImageFlexible(asset.image, path, width, height);
    asset.hasAlpha = RenderUtils::HasImage(asset.image) && RenderUtils::HasMeaningfulAlpha(asset.image);
}

void ResetImageAsset(ImageAsset& asset) {
    asset.image.Resize(0, 0);
    asset.hasAlpha = false;
}

void DrawIconShadow(int x, int y, int w, int h) {
    setlinecolor(RGB(26, 40, 68));
    rectangle(x - 2, y - 2, x + w + 2, y + h + 2);

    setlinecolor(RGB(12, 20, 36));
    rectangle(x + 1, y + 1, x + w + 5, y + h + 5);
}

void InitializeCharacterActionButtons() {
    g_characterActionRects[0] = { kRightCenterX - kNavButtonW - 16, kButtonsRowY, kRightCenterX - 16, kButtonsRowY + kNavButtonH };
    g_characterActionRects[1] = { kRightCenterX + 16, kButtonsRowY, kRightCenterX + 16 + kNavButtonW, kButtonsRowY + kNavButtonH };

    const int confirmY = kButtonsRowY + kNavButtonH + 8;
    g_characterActionRects[2] = { kRightCenterX - kConfirmButtonW / 2, confirmY, kRightCenterX + kConfirmButtonW / 2, confirmY + kConfirmButtonH };

    for (int i = 0; i < kCharacterActionCount; ++i) {
        g_characterActionHovered[i] = false;
    }
}

void BuildCharacterSelectBackgroundCache() {
    g_cachedBackground.Resize(GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT);
    SetWorkingImage(&g_cachedBackground);

    const int topR = 7, topG = 11, topB = 22;
    const int botR = 2, botG = 5, botB = 12;

    for (int y = 0; y < GAME_WINDOW_HEIGHT; ++y) {
        const int r = topR + (botR - topR) * y / GAME_WINDOW_HEIGHT;
        const int g = topG + (botG - topG) * y / GAME_WINDOW_HEIGHT;
        const int b = topB + (botB - topB) * y / GAME_WINDOW_HEIGHT;
        setlinecolor(RGB(r, g, b));
        line(0, y, GAME_WINDOW_WIDTH, y);
    }

    setlinecolor(RGB(44, 54, 80));
    rectangle(18, 18, GAME_WINDOW_WIDTH - 18, GAME_WINDOW_HEIGHT - 18);

    setlinecolor(RGB(30, 42, 68));
    line(kRightColumnX - kContentPadding / 2, 46, kRightColumnX - kContentPadding / 2, GAME_WINDOW_HEIGHT - 40);

    SetWorkingImage();
    g_backgroundCacheBuilt = true;
}

void DrawCharacterSelectBackground() {
    if (!g_backgroundCacheBuilt ||
        g_cachedBackground.getwidth() != GAME_WINDOW_WIDTH ||
        g_cachedBackground.getheight() != GAME_WINDOW_HEIGHT) {
        BuildCharacterSelectBackgroundCache();
    }

    putimage(0, 0, &g_cachedBackground);
}

void DrawCharacterSelectButtons() {
    for (int i = 0; i < kCharacterActionCount; ++i) {
        const RECT& rect = g_characterActionRects[i];
        const bool hovered = g_characterActionHovered[i];
        const COLORREF mainColor = hovered ? RGB(255, 255, 255) : RGB(208, 222, 244);
        const COLORREF shadowColor = hovered ? RGB(98, 128, 182) : RGB(36, 54, 90);

        MenuRenderUtils::DrawShadowedText(kCharacterActionLabels[i], rect, 28, mainColor, shadowColor);
    }
}

void DrawCharacterPlaceholder(int index) {
    setbkmode(TRANSPARENT);
    settextstyle(30, 0, _T("黑体"));
    settextcolor(WHITE);

    RECT titleRect = { kRightColumnX + 10, 94, GAME_WINDOW_WIDTH - 32, 150 };
    drawtext(GameData::GetCharacterDefinition(index).placeholderName, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    settextstyle(18, 0, _T("宋体"));
    settextcolor(RGB(180, 190, 210));
    RECT descRect = { kRightColumnX + 10, 165, GAME_WINDOW_WIDTH - 32, 260 };
    drawtext(_T("该角色资料正在整理中"), &descRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
}

void DrawCharacterProfile(int index) {
    if (index < 0 || index >= kCharacterCount) {
        DrawCharacterPlaceholder(0);
        return;
    }

    if (!RenderUtils::HasImage(g_characterPortraits[index].image)) {
        DrawCharacterPlaceholder(index);
        return;
    }

    const GameData::CharacterDefinition& character = GameData::GetCharacterDefinition(index);
    RenderUtils::DrawImageAuto(
        g_characterPortraits[index].image,
        g_characterPortraits[index].hasAlpha,
        kPortraitX,
        kPortraitY,
        kPortraitW,
        kPortraitH);

    RECT nameTitleRect = { kNameX, kNameY, kNameX + kNameW, kNameY + 104 };
    RECT nameSubRect = { kNameX, kNameY + 98, kNameX + kNameW, kNameY + kNameH };
    DrawTypeLabel(character.displayName, nameTitleRect, 74, RGB(246, 250, 255), RGB(30, 48, 80));
    DrawTypeLabel(character.subtitle, nameSubRect, 34, RGB(228, 238, 250), RGB(26, 40, 72));

    RECT weaponRect = { kWeaponX, kWeaponY, kWeaponX + kWeaponW, kWeaponY + kWeaponH };
    RECT personRect = { kPersonX, kPersonY, kPersonX + kPersonW, kPersonY + kPersonH };
    DrawTypeLabel(character.weaponLabel, weaponRect, 38, RGB(236, 244, 255), RGB(34, 52, 84));
    DrawTypeLabel(character.personLabel, personRect, 38, RGB(236, 244, 255), RGB(34, 52, 84));
    if (RenderUtils::HasImage(g_characterMoons[index].image)) {
        DrawIconShadow(kMoonX, kMoonY, kMoonW, kMoonH);
        RenderUtils::DrawImageAuto(
            g_characterMoons[index].image,
            g_characterMoons[index].hasAlpha,
            kMoonX,
            kMoonY,
            kMoonW,
            kMoonH);
    }

    RenderUtils::UpdateAnimatedGifFrame(g_characterGifs[index]);
    DrawIconShadow(kGifX, kGifY, kGifW, kGifH);
    RenderUtils::DrawAnimatedGif(g_characterGifs[index], kGifX, kGifY + kGifInnerOffsetY, kGifW, kGifH);
}

void LoadAssets() {
    for (int i = 0; i < kCharacterCount; ++i) {
        LoadImageAsset(g_characterPortraits[i], AssetPaths::GetCharacterPortraitPath(i));
        LoadImageAsset(g_characterMoons[i], AssetPaths::GetCharacterMoonPath(i), kMoonW, kMoonH);
        RenderUtils::LoadAnimatedGif(g_characterGifs[i], AssetPaths::GetCharacterSelectGifPath(i));
    }
}

void FreeAssets() {
    for (int i = 0; i < kCharacterCount; ++i) {
        ResetImageAsset(g_characterPortraits[i]);
        ResetImageAsset(g_characterMoons[i]);
        g_characterGifs[i].Reset();
    }

    g_cachedBackground.Resize(0, 0);
    g_backgroundCacheBuilt = false;
}

void UpdateActionHoverState(int x, int y) {
    constexpr int kHoverPadding = 20;

    for (int i = 0; i < kCharacterActionCount; ++i) {
        RECT hoverRect = g_characterActionRects[i];
        hoverRect.left -= kHoverPadding;
        hoverRect.right += kHoverPadding;
        hoverRect.top -= kHoverPadding;
        hoverRect.bottom += kHoverPadding;

        g_characterActionHovered[i] = RenderUtils::IsPointInRect(x, y, hoverRect);
    }
}

bool HandleActionClick(int x, int y) {
    if (RenderUtils::IsPointInRect(x, y, g_characterActionRects[0])) {
        g_selectedCharacterIndex = (g_selectedCharacterIndex - 1 + kCharacterCount) % kCharacterCount;
        return false;
    }
    if (RenderUtils::IsPointInRect(x, y, g_characterActionRects[1])) {
        g_selectedCharacterIndex = (g_selectedCharacterIndex + 1) % kCharacterCount;
        return false;
    }
    return RenderUtils::IsPointInRect(x, y, g_characterActionRects[2]);
}
}

namespace CharacterSelectModule {
void Initialize() {
    RenderUtils::AcquireGdiplus();
    MenuRenderUtils::AcquireMenuFont();
    LoadAssets();
    InitializeCharacterActionButtons();
}

void Shutdown() {
    FreeAssets();
    MenuRenderUtils::ReleaseMenuFont();
    RenderUtils::ReleaseGdiplus();
}

void Draw() {
    DrawCharacterSelectBackground();
    DrawCharacterProfile(g_selectedCharacterIndex);
    DrawCharacterSelectButtons();
}

void HandleMouseMove(int x, int y) {
    UpdateActionHoverState(x, y);
}

void HandleMouseClick(int x, int y, GameState& currentState) {
    if (HandleActionClick(x, y)) {
        currentState = GAME_START;
    }
}

int GetSelectedCharacterIndex() {
    return g_selectedCharacterIndex;
}
}

