#include "asset_paths.h"
#include "asset_paths_internal.h"

#include <array>
#include <string>

namespace AssetPaths {
namespace {

std::wstring CharacterPath(const wchar_t* characterName, const wchar_t* fileName) {
    return AssetPathsInternal::AssetPath({ L"character", characterName, fileName });
}

const std::array<std::wstring, AssetPathsInternal::kCharacterCount>& PortraitPaths() {
    static const std::array<std::wstring, AssetPathsInternal::kCharacterCount> paths = {
        CharacterPath(L"月神", L"月神立绘 .png"),
        CharacterPath(L"太阳神", L"太阳神立绘.png"),
        CharacterPath(L"爱神", L"爱神立绘.png")
    };
    return paths;
}

const std::array<std::wstring, AssetPathsInternal::kCharacterCount>& MoonPaths() {
    static const std::array<std::wstring, AssetPathsInternal::kCharacterCount> paths = {
        CharacterPath(L"月神", L"月相.png"),
        CharacterPath(L"太阳神", L"太阳神巨剑.png"),
        CharacterPath(L"爱神", L"十字架.png")
    };
    return paths;
}

const std::array<std::wstring, AssetPathsInternal::kCharacterCount>& CharacterSelectGifPaths() {
    static const std::array<std::wstring, AssetPathsInternal::kCharacterCount> paths = {
        CharacterPath(L"月神", L"月神.gif"),
        CharacterPath(L"太阳神", L"太阳神.gif"),
        CharacterPath(L"爱神", L"爱神.gif")
    };
    return paths;
}

const std::array<std::wstring, AssetPathsInternal::kCharacterCount>& CharacterWalkGifPaths() {
    static const std::array<std::wstring, AssetPathsInternal::kCharacterCount> paths = {
        CharacterPath(L"月神", L"月神行走.gif"),
        CharacterPath(L"太阳神", L"太阳神行走.gif"),
        CharacterPath(L"爱神", L"爱神行走.gif")
    };
    return paths;
}

const std::array<std::wstring, AssetPathsInternal::kCharacterCount>& CharacterDeathImagePaths() {
    static const std::array<std::wstring, AssetPathsInternal::kCharacterCount> paths = {
        CharacterPath(L"月神", L"月神死亡.png"),
        CharacterPath(L"太阳神", L"太阳神死亡.png"),
        CharacterPath(L"爱神", L"爱神死亡.png")
    };
    return paths;
}

const std::array<std::wstring, AssetPathsInternal::kCharacterCount>& UltimateCooldownIconPaths() {
    static const std::array<std::wstring, AssetPathsInternal::kCharacterCount> paths = {
        CharacterPath(L"月神", L"月神魔法弓.png"),
        CharacterPath(L"太阳神", L"太阳神盾牌.png"),
        CharacterPath(L"爱神", L"爱神星辉之剑.png")
    };
    return paths;
}

}  

const TCHAR* GetCharacterPortraitPath(int characterIndex) {
    return AssetPathsInternal::PathAtCharacter(PortraitPaths(), characterIndex);
}

const TCHAR* GetCharacterMoonPath(int characterIndex) {
    return AssetPathsInternal::PathAtCharacter(MoonPaths(), characterIndex);
}

const TCHAR* GetCharacterSelectGifPath(int characterIndex) {
    return AssetPathsInternal::PathAtCharacter(CharacterSelectGifPaths(), characterIndex);
}

const TCHAR* GetCharacterIdleGifPath(int characterIndex) {
    return GetCharacterSelectGifPath(characterIndex);
}

const TCHAR* GetCharacterWalkGifPath(int characterIndex) {
    return AssetPathsInternal::PathAtCharacter(CharacterWalkGifPaths(), characterIndex);
}

const TCHAR* GetCharacterDeathImagePath(int characterIndex) {
    return AssetPathsInternal::PathAtCharacter(CharacterDeathImagePaths(), characterIndex);
}

const TCHAR* GetApolloSlashGifPath() {
    static const std::wstring path = CharacterPath(L"太阳神", L"阿波罗挥砍.gif");
    return path.c_str();
}

const TCHAR* GetSunUltimateSwordQiPath() {
    static const std::wstring path = CharacterPath(L"太阳神", L"太阳神剑气.png");
    return path.c_str();
}

const TCHAR* GetSunCriticalHaloPath() {
    static const std::wstring path = CharacterPath(L"太阳神", L"太阳神暴击光环.png");
    return path.c_str();
}

const TCHAR* GetMoonRainArrowPath() {
    static const std::wstring path = CharacterPath(L"月神", L"月神箭雨.png");
    return path.c_str();
}

const TCHAR* GetMoonRainAimCirclePath() {
    static const std::wstring path = CharacterPath(L"月神", L"月神瞄准圈.png");
    return path.c_str();
}

const TCHAR* GetMoonUltimateFireballPath() {
    static const std::wstring path = CharacterPath(L"月神", L"月神火球.png");
    return path.c_str();
}

const TCHAR* GetMoonUltimateFireballAuraPath() {
    static const std::wstring path = CharacterPath(L"月神", L"月神火球光环.png");
    return path.c_str();
}

const TCHAR* GetMoonUltimateWavePath() {
    static const std::wstring path = CharacterPath(L"月神", L"月神能量波.png");
    return path.c_str();
}

const TCHAR* GetMoonMagicCagePath() {
    static const std::wstring path = CharacterPath(L"月神", L"月神魔法囚笼.png");
    return path.c_str();
}

const TCHAR* GetSunUltimateShieldPath() {
    static const std::wstring path = CharacterPath(L"太阳神", L"太阳神能量护盾.png");
    return path.c_str();
}

const TCHAR* GetLoveUltimateRecoverCirclePath() {
    static const std::wstring path = CharacterPath(L"爱神", L"爱神恢复法阵.png");
    return path.c_str();
}

const TCHAR* GetLoveUltimateBulletPath() {
    static const std::wstring path = CharacterPath(L"爱神", L"矩形子弹-黄.png");
    return path.c_str();
}

const TCHAR* GetLovePurifyCirclePath() {
    static const std::wstring path = CharacterPath(L"爱神", L"爱神净化法阵.png");
    return path.c_str();
}

const TCHAR* GetUltimateCooldownIconPath(int characterIndex) {
    return AssetPathsInternal::PathAtCharacter(UltimateCooldownIconPaths(), characterIndex);
}

const TCHAR* GetLoveParticlePath() {
    static const std::wstring path = CharacterPath(L"爱神", L"粒子.png");
    return path.c_str();
}

}  
