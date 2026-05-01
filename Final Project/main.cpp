#include <easyx.h>
#include <graphics.h>

#include "graphics_manager.h"
#include "animation.h"
#include "globals.h"
#include "menu_music.h"
#include "ui_manager.h"

int main()
{
    GraphicsManager::initialize();

    LogoAnimation::showStudioLogo();
    MenuMusic::Initialize();
    MenuMusic::OnStateChanged(MAIN_MENU);

    UIManager::initialize();
    UIManager::runMainLoop();

    MenuMusic::Shutdown();
    GraphicsManager::cleanup();
    return 0;
}
