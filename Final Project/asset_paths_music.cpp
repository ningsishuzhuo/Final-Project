#include "asset_paths.h"
#include "asset_paths_internal.h"

#include <string>

namespace AssetPaths {
namespace {

std::wstring MusicPath(const wchar_t* fileName) {
    return AssetPathsInternal::AssetPath({ L"music", fileName });
}

}  

const TCHAR* GetMainMenuMusicPath() {
    static const std::wstring path = MusicPath(L"大厅.mp3");
    return path.c_str();
}

const TCHAR* GetCharacterSelectMusicPath() {
    static const std::wstring path = MusicPath(L"角色选择.mp3");
    return path.c_str();
}

const TCHAR* GetNormalBattleMusicPath() {
    static const std::wstring path = MusicPath(L"普通战.mp3");
    return path.c_str();
}

const TCHAR* GetBossBattleMusicPath() {
    static const std::wstring path = MusicPath(L"boss战.mp3");
    return path.c_str();
}

}  
