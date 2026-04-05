#include "animation.h"
#include "graphics_manager.h"
#include "globals.h"
#include <windows.h>
#include <tchar.h>
#include <graphics.h>

IMAGE LogoAnimation::logo_images[3];

void LogoAnimation::showStudioLogo() {
    loadImages();
    playAnimation();
    freeImages();
}

void LogoAnimation::loadImages() {
    loadimage(&logo_images[0], _T("D:\\Users\\28780\\source\\repos\\Final Project\\assets\\logo\\logo1.png"));
    loadimage(&logo_images[1], _T("D:\\Users\\28780\\source\\repos\\Final Project\\assets\\logo\\logo2.png"));
    loadimage(&logo_images[2], _T("D:\\Users\\28780\\source\\repos\\Final Project\\assets\\logo\\logo3.png"));
}

void LogoAnimation::playAnimation() {
    // 定义动画序列：淡入 -> 完全显示 -> 淡出
    int frame_sequence[5] = { 0, 1, 2, 1, 0 };  // 对应logo1->logo2->logo3->logo2->logo1
    int delays[5] = { 100, 50, 1500, 50, 100 }; // 每帧停留时间（毫秒）

    // 播放动画
    for (int i = 0; i < 5; i++) {
        // 计算logo居中位置
        int x = (GAME_WINDOW_WIDTH - logo_images[frame_sequence[i]].getwidth()) / 2;
        int y = (GAME_WINDOW_HEIGHT - logo_images[frame_sequence[i]].getheight()) / 2;

        // 显示当前帧
        putimage(x, y, &logo_images[frame_sequence[i]]);

        // 延时
        Sleep(delays[i]);

        // 清除屏幕
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