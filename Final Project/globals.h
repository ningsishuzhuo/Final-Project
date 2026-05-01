#ifndef GLOBALS_H
#define GLOBALS_H

#include <tchar.h>
#include <graphics.h>


extern const int GAME_WINDOW_WIDTH;
extern const int GAME_WINDOW_HEIGHT;


enum GameState {
    LOGO_ANIMATION,
    MAIN_MENU,
    GAME_START,
    SETTINGS,
    CHARACTER_SELECT
};


struct Button {
    int x, y, width, height;
    TCHAR text[20];
    bool isHovered;
    IMAGE normalImage;  
    IMAGE hoverImage;   
};


enum BackgroundType {
    BG_MAIN_MENU,
    BG_GAME_START,
    BG_SETTINGS,
    BG_CHARACTER_SELECT
};

#endif
