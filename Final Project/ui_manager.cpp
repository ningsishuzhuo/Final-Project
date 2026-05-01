#include "ui_manager.h"
#include "character_select_module.h"
#include "game_settings_module.h"
#include "level_map.h"
#include "globals.h"
#include "asset_paths.h"
#include "menu_music.h"
#include "menu_render_utils.h"
#include "render_utils.h"
#include <graphics.h>
#include <tchar.h>
#include <windows.h>

namespace {
constexpr int kButtonCount = 3;
constexpr int kBackgroundCount = 4;
constexpr int kMenuButtonSpacing = 12;
constexpr int kDefaultButtonRenderWidth = 178;
constexpr int kDefaultButtonRenderHeight = 60;
constexpr int kFallbackButtonWidth = kDefaultButtonRenderWidth;
constexpr int kFallbackButtonHeight = kDefaultButtonRenderHeight;
constexpr int kHoverScalePercent = 104;

constexpr COLORREF kFallbackBackgroundColors[kBackgroundCount] = {
    RGB(32, 36, 48),
    RGB(28, 40, 56),
    RGB(36, 34, 44),
    RGB(40, 30, 30)
};

constexpr int kButtonRenderWidths[kButtonCount] = {
    178,
    178,
    164
};

constexpr int kButtonRenderHeights[kButtonCount] = {
    60,
    60,
    56
};

constexpr COLORREF kGlyphNormalColor = RGB(176, 132, 72);
constexpr COLORREF kGlyphHoverColor = RGB(205, 160, 96);

struct ButtonConfig {
    const TCHAR* text;
    GameState targetState;
};

constexpr ButtonConfig kMenuButtonConfigs[kButtonCount] = {
    { _T("开始游戏"), CHARACTER_SELECT },
    { _T("游戏设置"), SETTINGS },
    { _T("角色选择"), CHARACTER_SELECT }
};
}

GameState UIManager::current_state;
Button UIManager::menu_buttons[3];
IMAGE UIManager::background_image;

void UIManager::initialize() {
    current_state = MAIN_MENU;
    LevelMap::Initialize();
    UIManager::loadBackgrounds();
    UIManager::loadButtonImages();
    UIManager::initializeMenuButtons();
    CharacterSelectModule::Initialize();
    GameSettingsModule::Initialize();
}

void UIManager::loadBackgrounds() {
    RenderUtils::LoadImageFlexible(UIManager::background_image, AssetPaths::GetUiBackgroundPath(), 0, 0);
}

void UIManager::freeBackgrounds() {
    UIManager::background_image.Resize(0, 0);
}

void UIManager::drawBackground(BackgroundType bgType) {
    IMAGE& bg = UIManager::background_image;
    if (RenderUtils::HasImage(bg)) {
        putimage(0, 0, &bg);
        return;
    }

    int backgroundIndex = static_cast<int>(bgType);
    if (backgroundIndex < 0 || backgroundIndex >= kBackgroundCount) {
        backgroundIndex = 0;
    }
    setbkcolor(kFallbackBackgroundColors[backgroundIndex]);
    cleardevice();
}

void UIManager::loadButtonImages() {
    for (int i = 0; i < kButtonCount; ++i) {
        IMAGE sourceImage;

        const int normalW = kButtonRenderWidths[i];
        const int normalH = kButtonRenderHeights[i];
        const int hoverW = normalW * kHoverScalePercent / 100;
        const int hoverH = normalH * kHoverScalePercent / 100;

        const TCHAR* iconPath = AssetPaths::GetMenuButtonIconPath(i);
        RenderUtils::LoadImageFlexible(sourceImage, iconPath, normalW, normalH);

        MenuRenderUtils::ColorizeGlyphImage(sourceImage, UIManager::menu_buttons[i].normalImage, kGlyphNormalColor);

        IMAGE sourceHoverImage;
        RenderUtils::LoadImageFlexible(sourceHoverImage, iconPath, hoverW, hoverH);
        MenuRenderUtils::ColorizeGlyphImage(sourceHoverImage, UIManager::menu_buttons[i].hoverImage, kGlyphHoverColor);
    }
}

void UIManager::freeButtonImages() {
    for (int i = 0; i < kButtonCount; i++) {
        UIManager::menu_buttons[i].normalImage.Resize(0, 0);
        UIManager::menu_buttons[i].hoverImage.Resize(0, 0);
    }
}

void UIManager::initializeMenuButtons() {
    int totalHeight = 0;
    for (int i = 0; i < kButtonCount; ++i) {
        const int h = UIManager::menu_buttons[i].normalImage.getheight();
        totalHeight += (h > 0 ? h : kFallbackButtonHeight);
    }
    totalHeight += (kButtonCount - 1) * kMenuButtonSpacing;

    const int centeredY = (GAME_WINDOW_HEIGHT - totalHeight) / 2;
    const int startY = (centeredY < 250) ? 250 : centeredY;
    int currentY = startY;

    for (int i = 0; i < kButtonCount; ++i) {
        Button& button = UIManager::menu_buttons[i];
        _tcscpy_s(button.text, _countof(button.text), kMenuButtonConfigs[i].text);

        const int imageWidth = button.normalImage.getwidth();
        const int imageHeight = button.normalImage.getheight();
        button.width = imageWidth > 0 ? imageWidth : kFallbackButtonWidth;
        button.height = imageHeight > 0 ? imageHeight : kFallbackButtonHeight;

        button.x = (GAME_WINDOW_WIDTH - button.width) / 2;
        button.y = currentY;
        button.isHovered = false;

        currentY += button.height + kMenuButtonSpacing;
    }
}

bool UIManager::checkButtonHover(int x, int y, const Button& btn) {
    return (x >= btn.x && x <= btn.x + btn.width &&
        y >= btn.y && y <= btn.y + btn.height);
}

void UIManager::handleButtonClick(int x, int y) {
    for (int i = 0; i < kButtonCount; i++) {
        if (UIManager::checkButtonHover(x, y, UIManager::menu_buttons[i])) {
            current_state = kMenuButtonConfigs[i].targetState;
            if (current_state == SETTINGS) {
                GameSettingsModule::EnterSettings();
            }
            return;
        }
    }
}

void UIManager::drawMainMenu() {
    UIManager::drawBackground(BG_MAIN_MENU);

    for (int i = 0; i < kButtonCount; i++) {
        MenuRenderUtils::DrawMainMenuButton(UIManager::menu_buttons[i]);
    }
}

void UIManager::drawSettingsInterface() {
    GameSettingsModule::Draw();
}

void UIManager::drawCharacterSelectInterface() {
    CharacterSelectModule::Draw();
}

void UIManager::drawGameStartInterface() {
    LevelMap::Draw();
}

void UIManager::runMainLoop() {
    ExMessage msg;
    bool running = true;

    BeginBatchDraw();
    MenuMusic::OnStateChanged(current_state);

    while (running) {
        while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
            if (msg.message == WM_KEYDOWN) {
                if (current_state == GAME_START && LevelMap::HandleAnyKeyDown()) {
                    current_state = MAIN_MENU;
                    continue;
                }

                if (msg.vkcode == VK_F4) {
                    running = false;
                    break;
                }

                if (msg.vkcode == VK_ESCAPE) {
                    if (current_state == MAIN_MENU) {
                        running = false;
                    }
                    else {
                        current_state = MAIN_MENU;
                    }
                    continue;
                }
            }

            if (current_state == MAIN_MENU) {
                if (msg.message == WM_MOUSEMOVE) {
                    for (int i = 0; i < kButtonCount; i++) {
                        UIManager::menu_buttons[i].isHovered = UIManager::checkButtonHover(msg.x, msg.y, UIManager::menu_buttons[i]);
                    }
                }
                else if (msg.message == WM_LBUTTONDOWN) {
                    UIManager::handleButtonClick(msg.x, msg.y);
                }
            }
            else if (current_state == GAME_START) {
                if (msg.message == WM_LBUTTONDOWN) {
                    LevelMap::HandleMouseButtonDown(msg.x, msg.y);
                }
                else if (msg.message == WM_LBUTTONUP) {
                    LevelMap::HandleMouseButtonUp(msg.x, msg.y);
                }
            }
            else if (current_state == CHARACTER_SELECT) {
                if (msg.message == WM_MOUSEMOVE) {
                    CharacterSelectModule::HandleMouseMove(msg.x, msg.y);
                }
                else if (msg.message == WM_LBUTTONDOWN) {
                    const GameState prevState = current_state;
                    CharacterSelectModule::HandleMouseClick(msg.x, msg.y, current_state);
                    if (prevState == CHARACTER_SELECT && current_state == GAME_START) {
                        LevelMap::ResetCamera();
                    }
                }
            }
            else if (current_state == SETTINGS) {
                if (msg.message == WM_MOUSEMOVE) {
                    GameSettingsModule::HandleMouseMove(msg.x, msg.y);
                }
                else if (msg.message == WM_LBUTTONDOWN) {
                    GameSettingsModule::HandleMouseClick(msg.x, msg.y, current_state);
                }
            }
        }

        if (!running) {
            break;
        }

        MenuMusic::OnStateChanged(current_state);

        switch (current_state) {
        case MAIN_MENU:
            UIManager::drawMainMenu();
            break;
        case GAME_START:
            UIManager::drawGameStartInterface();
            break;
        case SETTINGS:
            UIManager::drawSettingsInterface();
            break;
        case CHARACTER_SELECT:
            UIManager::drawCharacterSelectInterface();
            break;
        default:
            UIManager::drawMainMenu();
            break;
        }

        FlushBatchDraw();
        Sleep(30);
    }

    EndBatchDraw();

    LevelMap::Shutdown();
    CharacterSelectModule::Shutdown();
    GameSettingsModule::Shutdown();
    UIManager::freeBackgrounds();
    UIManager::freeButtonImages();
}
