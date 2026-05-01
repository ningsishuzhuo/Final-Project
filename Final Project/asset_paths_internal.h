#ifndef ASSET_PATHS_INTERNAL_H
#define ASSET_PATHS_INTERNAL_H

#include "asset_paths.h"

#include <array>
#include <initializer_list>
#include <string>

namespace AssetPathsInternal {

constexpr int kCharacterCount = AssetPaths::CHARACTER_COUNT;

std::wstring JoinPath(const std::wstring& left, const wchar_t* right);
std::wstring AssetPath(std::initializer_list<const wchar_t*> parts);
const std::wstring& AssetRoot();
const TCHAR* PathAtCharacter(const std::array<std::wstring, kCharacterCount>& paths, int index);
const TCHAR* PathAtMenu(const std::array<std::wstring, 3>& paths, int index);

}  

#endif  
