#include <iostream>
#include <easyx.h>
#include <graphics.h>
#include <windows.h>
using namespace std;

#include "graphics_manager.h"
#include "animation.h"
#include "globals.h"

int main()
{
    // 初始化游戏窗口
    GraphicsManager::initialize();

    // 播放开屏logo动画
    LogoAnimation::showStudioLogo();

    // 保持窗口打开直到用户关闭
    while (1)
    {
        if (GetAsyncKeyState(VK_ESCAPE)) // 按ESC退出
            break;
    }

    // 清理资源
    GraphicsManager::cleanup();
    return 0;
}