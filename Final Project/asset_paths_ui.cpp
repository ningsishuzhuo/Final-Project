#include "asset_paths.h"
#include "asset_paths_internal.h"

#include <array>
#include <string>

namespace AssetPaths {
namespace {

std::wstring LogoPath(const wchar_t* fileName) {
    return AssetPathsInternal::AssetPath({ L"logo", fileName });
}

std::wstring UiPath(const wchar_t* fileName) {
    return AssetPathsInternal::AssetPath({ L"UI", fileName });
}

std::wstring StatePath(const wchar_t* fileName) {
    return AssetPathsInternal::AssetPath({ L"state", fileName });
}

std::wstring StatusBarPath(const wchar_t* fileName) {
    return AssetPathsInternal::AssetPath({ L"state", L"status bar", fileName });
}

std::wstring WaterPath(const wchar_t* fileName) {
    return AssetPathsInternal::AssetPath({ L"state", L"water", fileName });
}

const std::array<std::wstring, 3>& LogoPaths() {
    static const std::array<std::wstring, 3> paths = {
        LogoPath(L"logo1.png"),
        LogoPath(L"logo2.png"),
        LogoPath(L"logo3.png")
    };
    return paths;
}

const std::array<std::wstring, 3>& MenuButtonIconPaths() {
    static const std::array<std::wstring, 3> paths = {
        UiPath(L"start_1.png"),
        UiPath(L"settings_1.png"),
        UiPath(L"character_1.png")
    };
    return paths;
}

}  

const TCHAR* GetLogoPath(int index) {
    return AssetPathsInternal::PathAtMenu(LogoPaths(), index);
}

const TCHAR* GetUiBackgroundPath() {
    static const std::wstring path = UiPath(L"UI1.png");
    return path.c_str();
}

const TCHAR* GetMenuButtonIconPath(int index) {
    return AssetPathsInternal::PathAtMenu(MenuButtonIconPaths(), index);
}

const TCHAR* GetEliteBreakStatePath() {
    static const std::wstring path = StatePath(L"破防.gif");
    return path.c_str();
}

const TCHAR* GetPlayerHpIconPath() {
    static const std::wstring path = StatusBarPath(L"生命值.png");
    return path.c_str();
}

const TCHAR* GetPlayerArmorIconPath() {
    static const std::wstring path = StatusBarPath(L"护甲.png");
    return path.c_str();
}

const TCHAR* GetPlayerEnergyIconPath() {
    static const std::wstring path = StatusBarPath(L"能量.png");
    return path.c_str();
}

const TCHAR* GetEnergyDropIconPath() {
    static const std::wstring path = StatusBarPath(L"能量.png");
    return path.c_str();
}

const TCHAR* GetRecoverPotionDropIconPath() {
    static const std::wstring path = WaterPath(L"恢复药水.png");
    return path.c_str();
}

const TCHAR* GetEnergyPotionDropIconPath() {
    static const std::wstring path = WaterPath(L"能量药水.png");
    return path.c_str();
}

const TCHAR* GetLifePotionDropIconPath() {
    static const std::wstring path = WaterPath(L"生命药水.png");
    return path.c_str();
}

}  
