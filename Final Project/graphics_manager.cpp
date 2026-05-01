#include "graphics_manager.h"
#include "globals.h"
#include <graphics.h>
#include <windows.h>
#include <imm.h>

#pragma comment(lib, "Imm32.lib")

void GraphicsManager::initialize(){
    initgraph(GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT);

    HWND gameWindow = GetHWnd();
    if (gameWindow != nullptr) {
        ImmAssociateContext(gameWindow, (HIMC)NULL);
    }
}

void GraphicsManager::cleanup() {
    closegraph();
}

void GraphicsManager::clearScreen() {
    cleardevice();
}
