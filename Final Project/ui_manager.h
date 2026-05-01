#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "globals.h"
#include <graphics.h>

class UIManager {
public:
    static void initialize();
    static void runMainLoop();

    static void loadBackgrounds();
    static void freeBackgrounds();
    static void drawBackground(BackgroundType bgType);

private:
    static void initializeMenuButtons();
    static void drawMainMenu();
    static bool checkButtonHover(int x, int y, const Button& btn);
    static void handleButtonClick(int x, int y);

    static void drawGameStartInterface();
    static void drawSettingsInterface();
    static void drawCharacterSelectInterface();

    static GameState current_state;
    static Button menu_buttons[3];
    static IMAGE background_image;

    static void loadButtonImages();
    static void freeButtonImages();
};

#endif
