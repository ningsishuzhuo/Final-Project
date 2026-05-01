#include "asset_paths.h"
#include "asset_paths_internal.h"

#include <windows.h>

#include <initializer_list>
#include <string>

namespace AssetPathsInternal {
namespace {

std::wstring GetExecutableDirectory() {
    std::wstring buffer(MAX_PATH, L'\0');

    for (;;) {
        DWORD written = GetModuleFileNameW(nullptr, &buffer[0], static_cast<DWORD>(buffer.size()));
        if (written == 0) {
            return L".";
        }

        if (written < buffer.size() - 1) {
            buffer.resize(written);
            break;
        }

        buffer.resize(buffer.size() * 2);
    }

    const size_t slash = buffer.find_last_of(L"\\/");
    return (slash == std::wstring::npos) ? L"." : buffer.substr(0, slash);
}

std::wstring ParentDirectory(const std::wstring& path) {
    const size_t slash = path.find_last_of(L"\\/");
    return (slash == std::wstring::npos) ? path : path.substr(0, slash);
}

}  

std::wstring JoinPath(const std::wstring& left, const wchar_t* right) {
    if (left.empty()) {
        return std::wstring(right);
    }

    if (left.back() == L'\\' || left.back() == L'/') {
        return left + right;
    }

    return left + L'\\' + right;
}

std::wstring AssetPath(std::initializer_list<const wchar_t*> parts) {
    std::wstring path = AssetRoot();
    for (const wchar_t* part : parts) {
        path = JoinPath(path, part);
    }
    return path;
}

const std::wstring& AssetRoot() {
    static const std::wstring root = []() {
        std::wstring dir = GetExecutableDirectory();
        dir = ParentDirectory(dir);
        dir = ParentDirectory(dir);
        return JoinPath(dir, L"assets");
    }();
    return root;
}

const TCHAR* PathAtCharacter(const std::array<std::wstring, kCharacterCount>& paths, int index) {
    if (index < 0 || index >= static_cast<int>(paths.size())) {
        return nullptr;
    }
    return paths[index].c_str();
}

const TCHAR* PathAtMenu(const std::array<std::wstring, 3>& paths, int index) {
    if (index < 0 || index >= static_cast<int>(paths.size())) {
        return nullptr;
    }
    return paths[index].c_str();
}

}  

namespace AssetPaths {

const TCHAR* GetAssetRoot() {
    return AssetPathsInternal::AssetRoot().c_str();
}

const TCHAR* GetTypeFontPath() {
    static const std::wstring path = AssetPathsInternal::AssetPath({ L"Type.ttf" });
    return path.c_str();
}

}  
