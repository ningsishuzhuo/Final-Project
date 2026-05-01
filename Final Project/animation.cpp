#include "animation.h"
#include "asset_paths.h"
#include "graphics_manager.h"
#include "globals.h"
#include <windows.h>
#include <tchar.h>
#include <graphics.h>

namespace {
constexpr int kLogoFrameCount = 3;
}

IMAGE LogoAnimation::logo_images[3];

void LogoAnimation::showStudioLogo() {
    loadImages();
    playAnimation();
    freeImages();
}

void LogoAnimation::loadImages() {
    for (int i = 0; i < kLogoFrameCount; ++i) {
        loadimage(&logo_images[i], AssetPaths::GetLogoPath(i));
    }
}

void LogoAnimation::playAnimation() {
    constexpr int kFrameSequence[5] = { 0, 1, 2, 1, 0 };
    constexpr int kFrameDelays[5] = { 100, 50, 1500, 50, 100 };

    for (int i = 0; i < 5; i++) {
        const IMAGE& frame = logo_images[kFrameSequence[i]];
        const int width = frame.getwidth();
        const int height = frame.getheight();
        if (width > 0 && height > 0) {
            const int x = (GAME_WINDOW_WIDTH - width) / 2;
            const int y = (GAME_WINDOW_HEIGHT - height) / 2;
            putimage(x, y, &frame);
        }

        Sleep(kFrameDelays[i]);

        if (i < 4) {
            GraphicsManager::clearScreen();
        }
    }
}

void LogoAnimation::freeImages() {
    for (int i = 0; i < 3; i++) {
        logo_images[i].Resize(0, 0);
    }
}
