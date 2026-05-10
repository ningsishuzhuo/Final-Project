#include "menu_render_utils.h"

#include "asset_paths.h"
#include "globals.h"
#include "render_utils.h"

#include <tchar.h>
#include <windows.h>
#include <wingdi.h>

namespace MenuRenderUtils {
namespace {

constexpr COLORREF kGlyphStrokeColor = RGB(45, 30, 16);
constexpr COLORREF kGlyphShadowColor = RGB(8, 14, 22);

constexpr int kFallbackButtonWidth = 178;
constexpr int kFallbackButtonHeight = 60;
constexpr const TCHAR* kMenuFontFace = _T("Type");
constexpr const TCHAR* kFallbackFontFace = _T("黑体");

int g_menuFontUsers = 0;
bool g_menuFontLoaded = false;

bool IsNearWhite(COLORREF color) {
    return GetRValue(color) > 245 && GetGValue(color) > 245 && GetBValue(color) > 245;
}

void DrawGlyphTransparent(const IMAGE& glyph, int dstX, int dstY) {
    const int width = glyph.getwidth();
    const int height = glyph.getheight();
    if (width <= 0 || height <= 0) {
        return;
    }

    DWORD* glyphBuffer = GetImageBuffer(const_cast<IMAGE*>(&glyph));

    auto isGlyphAt = [&](int x, int y) -> bool {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return false;
        }
        return !IsNearWhite(glyphBuffer[y * width + x]);
    };

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (!isGlyphAt(x, y)) {
                continue;
            }

            const int shadowX = x + 1;
            const int shadowY = y + 1;
            if (isGlyphAt(shadowX, shadowY)) {
                continue;
            }

            const int px = dstX + shadowX;
            const int py = dstY + shadowY;
            if (px >= 0 && px < GAME_WINDOW_WIDTH && py >= 0 && py < GAME_WINDOW_HEIGHT) {
                putpixel(px, py, kGlyphShadowColor);
            }
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (isGlyphAt(x, y)) {
                continue;
            }

            bool nearGlyph = false;
            for (int oy = -1; oy <= 1 && !nearGlyph; ++oy) {
                for (int ox = -1; ox <= 1; ++ox) {
                    if (ox == 0 && oy == 0) {
                        continue;
                    }
                    if (isGlyphAt(x + ox, y + oy)) {
                        nearGlyph = true;
                        break;
                    }
                }
            }

            if (!nearGlyph) {
                continue;
            }

            const int px = dstX + x;
            const int py = dstY + y;
            if (px >= 0 && px < GAME_WINDOW_WIDTH && py >= 0 && py < GAME_WINDOW_HEIGHT) {
                putpixel(px, py, kGlyphStrokeColor);
            }
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const COLORREF color = glyphBuffer[y * width + x];
            if (IsNearWhite(color)) {
                continue;
            }

            const int px = dstX + x;
            const int py = dstY + y;
            if (px >= 0 && px < GAME_WINDOW_WIDTH && py >= 0 && py < GAME_WINDOW_HEIGHT) {
                putpixel(px, py, color);
            }
        }
    }
}

void DrawFallbackButton(const Button& button) {
    setlinecolor(WHITE);
    setfillcolor(button.isHovered ? RGB(88, 130, 190) : RGB(60, 84, 120));
    solidrectangle(button.x, button.y, button.x + button.width, button.y + button.height);

    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle(22, 0, _T("黑体"));

    const int textW = textwidth(button.text);
    const int textH = textheight(button.text);
    outtextxy(button.x + (button.width - textW) / 2, button.y + (button.height - textH) / 2, button.text);
}

const TCHAR* GetMenuFontFace() {
    return g_menuFontLoaded ? kMenuFontFace : kFallbackFontFace;
}

}  

void ColorizeGlyphImage(const IMAGE& source, IMAGE& target, COLORREF baseColor) {
    const int width = source.getwidth();
    const int height = source.getheight();
    if (width <= 0 || height <= 0) {
        target.Resize(0, 0);
        return;
    }

    target.Resize(width, height);

    DWORD* sourceBuffer = GetImageBuffer(const_cast<IMAGE*>(&source));
    DWORD* targetBuffer = GetImageBuffer(&target);
    const int pixelCount = width * height;

    for (int i = 0; i < pixelCount; ++i) {
        const COLORREF src = sourceBuffer[i];
        const int r = GetRValue(src);
        const int g = GetGValue(src);
        const int b = GetBValue(src);

        const int maxValue = (r > g ? (r > b ? r : b) : (g > b ? g : b));
        const int minValue = (r < g ? (r < b ? r : b) : (g < b ? g : b));

        if (maxValue > 245 && minValue > 210) {
            targetBuffer[i] = WHITE;
            continue;
        }

        const int darkness = 255 - maxValue;
        if (darkness < 18) {
            targetBuffer[i] = WHITE;
            continue;
        }

        const int factor = 145 + darkness * 110 / 255;
        const int nr = GetRValue(baseColor) * factor / 255;
        const int ng = GetGValue(baseColor) * factor / 255;
        const int nb = GetBValue(baseColor) * factor / 255;
        targetBuffer[i] = RGB(nr, ng, nb);
    }
}

void DrawMainMenuButton(const Button& button) {
    if (RenderUtils::HasImage(button.normalImage) && RenderUtils::HasImage(button.hoverImage)) {
        const IMAGE& glyph = button.isHovered ? button.hoverImage : button.normalImage;
        int iconX = button.x;
        int iconY = button.y;

        if (button.isHovered) {
            iconX -= (glyph.getwidth() - button.width) / 2;
            iconY -= (glyph.getheight() - button.height) / 2 + 1;
        }

        DrawGlyphTransparent(glyph, iconX, iconY);
        return;
    }

    Button fallbackButton = button;
    if (fallbackButton.width <= 0) {
        fallbackButton.width = kFallbackButtonWidth;
    }
    if (fallbackButton.height <= 0) {
        fallbackButton.height = kFallbackButtonHeight;
    }
    DrawFallbackButton(fallbackButton);
}

void AcquireMenuFont() {
    ++g_menuFontUsers;
    if (g_menuFontLoaded) {
        return;
    }

    const int added = AddFontResourceEx(AssetPaths::GetTypeFontPath(), FR_PRIVATE, nullptr);
    g_menuFontLoaded = (added > 0);
}

void ReleaseMenuFont() {
    if (g_menuFontUsers <= 0) {
        return;
    }

    --g_menuFontUsers;
    if (g_menuFontUsers == 0 && g_menuFontLoaded) {
        RemoveFontResourceEx(AssetPaths::GetTypeFontPath(), FR_PRIVATE, nullptr);
        g_menuFontLoaded = false;
    }
}

void DrawShadowedText(
    const TCHAR* text,
    const RECT& rect,
    int fontSize,
    COLORREF mainColor,
    COLORREF shadowColor,
    int shadowOffset,
    UINT format) {
    setbkmode(TRANSPARENT);
    settextstyle(fontSize, 0, GetMenuFontFace());

    RECT shadowRect = rect;
    OffsetRect(&shadowRect, shadowOffset, shadowOffset);
    settextcolor(shadowColor);
    drawtext(text, &shadowRect, format);

    RECT textRect = rect;
    settextcolor(mainColor);
    drawtext(text, &textRect, format);
}

}  
