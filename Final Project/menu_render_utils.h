#ifndef MENU_RENDER_UTILS_H
#define MENU_RENDER_UTILS_H

#include "globals.h"

namespace MenuRenderUtils {

void ColorizeGlyphImage(const IMAGE& source, IMAGE& target, COLORREF baseColor);
void DrawMainMenuButton(const Button& button);

}  

#endif
