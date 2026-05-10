#ifndef MENU_RENDER_UTILS_H
#define MENU_RENDER_UTILS_H

#include "globals.h"

namespace MenuRenderUtils {

void ColorizeGlyphImage(const IMAGE& source, IMAGE& target, COLORREF baseColor);
void DrawMainMenuButton(const Button& button);
void AcquireMenuFont();
void ReleaseMenuFont();
void DrawShadowedText(
    const TCHAR* text,
    const RECT& rect,
    int fontSize,
    COLORREF mainColor,
    COLORREF shadowColor,
    int shadowOffset = 1,
    UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE);

}  

#endif
