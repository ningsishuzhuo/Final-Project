#include "menu_music.h"

#include "asset_paths.h"
#include "level_map.h"

#include <mmsystem.h>
#include <string>

#pragma comment(lib, "winmm.lib")

namespace MenuMusic {
namespace {

constexpr const wchar_t* kBgmAlias = L"final_project_menu_bgm";

enum class MusicTrack {
    None,
    MainMenu,
    CharacterSelect,
    NormalBattle,
    BossBattle
};

constexpr int kVolumeMin = 0;
constexpr int kVolumeMax = 10;
constexpr int kVolumeBaselineLevel = 5;
constexpr int kMciVolumeMin = 0;
constexpr int kMciVolumeMax = 1000;

bool g_initialized = false;
bool g_musicOpened = false;
bool g_menuMusicPlaying = false;
MusicTrack g_currentTrack = MusicTrack::None;
int g_backgroundVolumeLevel = kVolumeBaselineLevel;
int g_gameVolumeLevel = kVolumeBaselineLevel;

int ClampInt(int value, int low, int high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

void CloseMusicIfNeeded() {
    if (!g_musicOpened) {
        g_menuMusicPlaying = false;
        g_currentTrack = MusicTrack::None;
        return;
    }

    std::wstring stopCmd = L"stop ";
    stopCmd += kBgmAlias;
    mciSendStringW(stopCmd.c_str(), nullptr, 0, nullptr);

    std::wstring closeCmd = L"close ";
    closeCmd += kBgmAlias;
    mciSendStringW(closeCmd.c_str(), nullptr, 0, nullptr);

    g_musicOpened = false;
    g_menuMusicPlaying = false;
    g_currentTrack = MusicTrack::None;
}

const TCHAR* TrackPath(MusicTrack track) {
    switch (track) {
    case MusicTrack::MainMenu:
        return AssetPaths::GetMainMenuMusicPath();
    case MusicTrack::CharacterSelect:
        return AssetPaths::GetCharacterSelectMusicPath();
    case MusicTrack::NormalBattle:
        return AssetPaths::GetNormalBattleMusicPath();
    case MusicTrack::BossBattle:
        return AssetPaths::GetBossBattleMusicPath();
    default:
        return nullptr;
    }
}

bool IsBackgroundTrack(MusicTrack track) {
    return track == MusicTrack::MainMenu || track == MusicTrack::CharacterSelect;
}

int TrackBaselineVolume(MusicTrack track) {
    switch (track) {
    case MusicTrack::NormalBattle:
        return 940;
    case MusicTrack::MainMenu:
    case MusicTrack::CharacterSelect:
    case MusicTrack::BossBattle:
        return 880;
    default:
        return 0;
    }
}

int TrackLevelVolume(MusicTrack track) {
    return IsBackgroundTrack(track) ? g_backgroundVolumeLevel : g_gameVolumeLevel;
}

int ResolveTrackOutputVolume(MusicTrack track) {
    const int baseline = TrackBaselineVolume(track);
    const int level = TrackLevelVolume(track);
    const int scaled = baseline * level / kVolumeBaselineLevel;
    return ClampInt(scaled, kMciVolumeMin, kMciVolumeMax);
}

void ApplyTrackVolume(MusicTrack track) {
    if (!g_musicOpened || track == MusicTrack::None) {
        return;
    }

    std::wstring volumeCmd = L"setaudio ";
    volumeCmd += kBgmAlias;
    volumeCmd += L" volume to ";
    volumeCmd += std::to_wstring(ResolveTrackOutputVolume(track));
    mciSendStringW(volumeCmd.c_str(), nullptr, 0, nullptr);
}

void PlayMusicLoop(MusicTrack track) {
    if (track == MusicTrack::None) {
        CloseMusicIfNeeded();
        return;
    }

    if (g_menuMusicPlaying && g_currentTrack == track) {
        ApplyTrackVolume(track);
        return;
    }

    CloseMusicIfNeeded();

    const TCHAR* path = TrackPath(track);
    if (path == nullptr || path[0] == _T('\0')) {
        return;
    }

    std::wstring openCmd = L"open \"";
    openCmd += path;
    openCmd += L"\" type mpegvideo alias ";
    openCmd += kBgmAlias;

    if (mciSendStringW(openCmd.c_str(), nullptr, 0, nullptr) != 0) {
        CloseMusicIfNeeded();
        return;
    }
    g_musicOpened = true;

    ApplyTrackVolume(track);

    std::wstring playCmd = L"play ";
    playCmd += kBgmAlias;
    playCmd += L" repeat";

    if (mciSendStringW(playCmd.c_str(), nullptr, 0, nullptr) != 0) {
        CloseMusicIfNeeded();
        return;
    }

    g_menuMusicPlaying = true;
    g_currentTrack = track;
}

MusicTrack ResolveTrack(GameState state) {
    if (state == GAME_START) {
        return LevelMap::IsBossBattleActive() ? MusicTrack::BossBattle : MusicTrack::NormalBattle;
    }
    if (state == CHARACTER_SELECT) {
        return MusicTrack::CharacterSelect;
    }
    if (state == MAIN_MENU || state == SETTINGS) {
        return MusicTrack::MainMenu;
    }
    return MusicTrack::None;
}

}  

void Initialize() {
    if (g_initialized) {
        return;
    }
    g_initialized = true;
    g_musicOpened = false;
    g_menuMusicPlaying = false;
    g_currentTrack = MusicTrack::None;
    g_backgroundVolumeLevel = kVolumeBaselineLevel;
    g_gameVolumeLevel = kVolumeBaselineLevel;
}

void Shutdown() {
    CloseMusicIfNeeded();
    g_initialized = false;
}

void OnStateChanged(GameState state) {
    if (!g_initialized) {
        Initialize();
    }

    PlayMusicLoop(ResolveTrack(state));
}

int GetBackgroundVolumeLevel() {
    return g_backgroundVolumeLevel;
}

int GetGameVolumeLevel() {
    return g_gameVolumeLevel;
}

void AdjustBackgroundVolume(int delta) {
    g_backgroundVolumeLevel = ClampInt(g_backgroundVolumeLevel + delta, kVolumeMin, kVolumeMax);
    if (g_musicOpened && IsBackgroundTrack(g_currentTrack)) {
        ApplyTrackVolume(g_currentTrack);
    }
}

void AdjustGameVolume(int delta) {
    g_gameVolumeLevel = ClampInt(g_gameVolumeLevel + delta, kVolumeMin, kVolumeMax);
    if (g_musicOpened && !IsBackgroundTrack(g_currentTrack) && g_currentTrack != MusicTrack::None) {
        ApplyTrackVolume(g_currentTrack);
    }
}

}  
