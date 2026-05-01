#include "asset_paths.h"
#include "asset_paths_internal.h"

#include <string>

namespace AssetPaths {
namespace {

std::wstring IcefieldCharacterPath(const wchar_t* folderName, const wchar_t* fileName) {
    return AssetPathsInternal::AssetPath({ L"character", L"冰原", folderName, fileName });
}

std::wstring IcefieldItemPath(const wchar_t* fileName) {
    return AssetPathsInternal::AssetPath({ L"character", L"冰原", L"item", fileName });
}

}  

const TCHAR* GetMinerEnemyIdlePath() {
    static const std::wstring path = IcefieldCharacterPath(L"矿工", L"精英矿工待机.gif");
    return path.c_str();
}

const TCHAR* GetMinerEnemyWalkPath() {
    static const std::wstring path = IcefieldCharacterPath(L"矿工", L"精英矿工行走.gif");
    return path.c_str();
}

const TCHAR* GetMinerEnemyDeathPath() {
    static const std::wstring path = IcefieldCharacterPath(L"矿工", L"精英矿工死亡.png");
    return path.c_str();
}

const TCHAR* GetMinerEnemyBulletPath() {
    static const std::wstring path = IcefieldCharacterPath(L"矿工", L"矩形子弹_蓝.png");
    return path.c_str();
}

const TCHAR* GetSnowApeEnemyIdlePath() {
    static const std::wstring path = IcefieldCharacterPath(L"大雪猿", L"大雪猿待机.gif");
    return path.c_str();
}

const TCHAR* GetSnowApeEnemyWalkPath() {
    static const std::wstring path = IcefieldCharacterPath(L"大雪猿", L"大雪猿行走.gif");
    return path.c_str();
}

const TCHAR* GetSnowApeEnemyDeathPath() {
    static const std::wstring path = IcefieldCharacterPath(L"大雪猿", L"大雪猿死亡.png");
    return path.c_str();
}

const TCHAR* GetSnowApeShockwavePath() {
    static const std::wstring path = IcefieldCharacterPath(L"大雪猿", L"蓝色震荡波20.gif");
    return path.c_str();
}

const TCHAR* GetSnowApeKingEnemyIdlePath() {
    static const std::wstring path = IcefieldCharacterPath(L"雪山大猿王", L"雪山大猿王待机.gif");
    return path.c_str();
}

const TCHAR* GetSnowApeKingEnemyWalkPath() {
    static const std::wstring path = IcefieldCharacterPath(L"雪山大猿王", L"雪山大猿王行走.gif");
    return path.c_str();
}

const TCHAR* GetSnowApeKingEnemyDeathPath() {
    static const std::wstring path = IcefieldCharacterPath(L"雪山大猿王", L"雪山大猿王死亡.png");
    return path.c_str();
}

const TCHAR* GetSnowApeKingEnemyBulletPath() {
    static const std::wstring path = IcefieldCharacterPath(L"雪山大猿王", L"十字形子弹-敌方.png");
    return path.c_str();
}

const TCHAR* GetSnowApeKingEnemyJumpPath() {
    static const std::wstring path = IcefieldCharacterPath(L"雪山大猿王", L"雪山大猿王跳.gif");
    return path.c_str();
}

const TCHAR* GetSnowApeKingEnemyRoarPath() {
    static const std::wstring path = IcefieldCharacterPath(L"雪山大猿王", L"雪山大猿王怒吼.gif");
    return path.c_str();
}

const TCHAR* GetSnowApeKingIceSpikePath() {
    static const std::wstring path = IcefieldCharacterPath(L"雪山大猿王", L"冰刺.png");
    return path.c_str();
}

const TCHAR* GetIcefieldWallPath() {
    static const std::wstring path = IcefieldItemPath(L"冰原墙壁.png");
    return path.c_str();
}

const TCHAR* GetIcefieldRockPath() {
    static const std::wstring path = IcefieldItemPath(L"冰原大石头.png");
    return path.c_str();
}

const TCHAR* GetIcefieldMinecartPath() {
    static const std::wstring path = IcefieldItemPath(L"冰原矿车.png");
    return path.c_str();
}

const TCHAR* GetIcefieldBrokenMinecartPath() {
    static const std::wstring path = IcefieldItemPath(L"冰原损坏的矿车.png");
    return path.c_str();
}

const TCHAR* GetIcefieldOreWallPath() {
    static const std::wstring path = IcefieldItemPath(L"冰原矿石墙壁.png");
    return path.c_str();
}

const TCHAR* GetIcefieldRailPath() {
    static const std::wstring path = IcefieldItemPath(L"冰原铁轨.png");
    return path.c_str();
}

const TCHAR* GetIcefieldCratePath() {
    static const std::wstring path = IcefieldItemPath(L"冰原箱子.png");
    return path.c_str();
}

const TCHAR* GetIcefieldTorchPath() {
    static const std::wstring path = IcefieldItemPath(L"火炬.gif");
    return path.c_str();
}

}  
