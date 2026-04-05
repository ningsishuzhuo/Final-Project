#include "graphics_manager.h"
#include "globals.h"
#include <graphics.h>

void GraphicsManager::initialize(){
    initgraph(GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT);
}

void GraphicsManager::cleanup() {
    closegraph();
}

void GraphicsManager::clearScreen() {
    cleardevice();
}