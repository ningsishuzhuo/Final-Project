#include "render_utils.h"

#include <cmath>
#include <string>
#include <unordered_map>
#include <tchar.h>

namespace RenderUtils {
namespace {

ULONG_PTR g_gdiplusToken = 0;
bool g_gdiplusStarted = false;
int g_gdiplusRefCount = 0;
std::unordered_map<HBITMAP, std::unique_ptr<Gdiplus::Bitmap>> g_bitmapCache;
std::unordered_map<std::wstring, std::unique_ptr<Gdiplus::Image>> g_fileImageCache;

struct CachedVisibleBounds {
    bool valid = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

std::unordered_map<std::wstring, CachedVisibleBounds> g_fileVisibleBoundsCache;

unsigned char GetAlpha(DWORD c) {
    return static_cast<unsigned char>((c >> 24) & 0xFF);
}

Gdiplus::Bitmap* GetCachedBitmap(const IMAGE& image) {
    HDC srcDc = GetImageHDC(const_cast<IMAGE*>(&image));
    HBITMAP srcBitmap = static_cast<HBITMAP>(GetCurrentObject(srcDc, OBJ_BITMAP));
    if (srcBitmap == nullptr) {
        return nullptr;
    }

    const auto cached = g_bitmapCache.find(srcBitmap);
    if (cached != g_bitmapCache.end()) {
        return cached->second.get();
    }

    std::unique_ptr<Gdiplus::Bitmap> bitmap(Gdiplus::Bitmap::FromHBITMAP(srcBitmap, nullptr));
    if (bitmap == nullptr || bitmap->GetLastStatus() != Gdiplus::Ok) {
        return nullptr;
    }

    Gdiplus::Bitmap* bitmapPtr = bitmap.get();
    g_bitmapCache.emplace(srcBitmap, std::move(bitmap));
    return bitmapPtr;
}

Gdiplus::Image* GetCachedFileImage(const TCHAR* path) {
    if (path == nullptr || path[0] == _T('\0') || !g_gdiplusStarted) {
        return nullptr;
    }

    const std::wstring key(path);
    const auto cached = g_fileImageCache.find(key);
    if (cached != g_fileImageCache.end()) {
        return cached->second.get();
    }

    std::unique_ptr<Gdiplus::Image> image(Gdiplus::Image::FromFile(path, FALSE));
    if (image == nullptr || image->GetLastStatus() != Gdiplus::Ok) {
        return nullptr;
    }

    Gdiplus::Image* imagePtr = image.get();
    g_fileImageCache.emplace(key, std::move(image));
    return imagePtr;
}

void DrawRotatedGdiplusImage(
    Gdiplus::Image* image,
    int centerX,
    int centerY,
    int dstW,
    int dstH,
    float angleDegrees,
    Gdiplus::ImageAttributes* attrs) {
    if (image == nullptr || !g_gdiplusStarted) {
        return;
    }

    Gdiplus::Graphics graphics(GetImageHDC());
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

    graphics.TranslateTransform(static_cast<Gdiplus::REAL>(centerX), static_cast<Gdiplus::REAL>(centerY));
    graphics.RotateTransform(angleDegrees);
    graphics.TranslateTransform(static_cast<Gdiplus::REAL>(-dstW) / 2.0f, static_cast<Gdiplus::REAL>(-dstH) / 2.0f);

    Gdiplus::ImageAttributes localAttrs;
    Gdiplus::ImageAttributes* effectiveAttrs = attrs;
    if (effectiveAttrs == nullptr) {
        localAttrs.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
        effectiveAttrs = &localAttrs;
    }
    else {
        effectiveAttrs->SetWrapMode(Gdiplus::WrapModeTileFlipXY);
    }

    graphics.DrawImage(
        image,
        Gdiplus::Rect(0, 0, dstW, dstH),
        0,
        0,
        image->GetWidth(),
        image->GetHeight(),
        Gdiplus::UnitPixel,
        effectiveAttrs);

    graphics.ResetTransform();
}

}  

bool HasImage(const IMAGE& image) {
    return image.getwidth() > 0 && image.getheight() > 0;
}

ULONGLONG NowTickMs() {
    using GetTickCount64Proc = ULONGLONG(WINAPI*)();
    static GetTickCount64Proc proc = reinterpret_cast<GetTickCount64Proc>(
        GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetTickCount64"));
    if (proc != nullptr) {
        return proc();
    }
    return static_cast<ULONGLONG>(GetTickCount());
}

bool HasMeaningfulAlpha(const IMAGE& image) {
    const int width = image.getwidth();
    const int height = image.getheight();
    if (width <= 0 || height <= 0) {
        return false;
    }

    DWORD* buffer = GetImageBuffer(const_cast<IMAGE*>(&image));
    const int pixelCount = width * height;
    int alphaPixels = 0;

    for (int i = 0; i < pixelCount; ++i) {
        if (GetAlpha(buffer[i]) > 8) {
            ++alphaPixels;
        }
    }

    return alphaPixels > pixelCount / 200;
}

void AcquireGdiplus() {
    ++g_gdiplusRefCount;
    if (g_gdiplusStarted) {
        return;
    }

    Gdiplus::GdiplusStartupInput input;
    if (Gdiplus::GdiplusStartup(&g_gdiplusToken, &input, nullptr) == Gdiplus::Ok) {
        g_gdiplusStarted = true;
        return;
    }

    --g_gdiplusRefCount;
}

void ReleaseGdiplus() {
    if (g_gdiplusRefCount <= 0) {
        return;
    }

    --g_gdiplusRefCount;
    if (g_gdiplusRefCount == 0 && g_gdiplusStarted) {
        g_bitmapCache.clear();
        g_fileImageCache.clear();
        g_fileVisibleBoundsCache.clear();
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        g_gdiplusToken = 0;
        g_gdiplusStarted = false;
    }
}

bool IsGdiplusReady() {
    return g_gdiplusStarted;
}

void LoadImageFlexible(IMAGE& dst, const TCHAR* path, int targetW, int targetH) {
    dst.Resize(0, 0);

    if (path == nullptr) {
        return;
    }

    if (targetW > 0 && targetH > 0) {
        loadimage(&dst, path, targetW, targetH, true);
        if (HasImage(dst)) {
            return;
        }

        dst.Resize(0, 0);
        loadimage(&dst, path, targetW, targetH, false);
        if (HasImage(dst)) {
            return;
        }
    }

    dst.Resize(0, 0);
    loadimage(&dst, path);
}


void DrawImageAuto(const IMAGE& image, bool hasMeaningfulAlpha, int dstX, int dstY, int dstW, int dstH) {
    const int srcW = image.getwidth();
    const int srcH = image.getheight();
    if (srcW <= 0 || srcH <= 0) {
        return;
    }

    const int drawW = dstW > 0 ? dstW : srcW;
    const int drawH = dstH > 0 ? dstH : srcH;

    if (hasMeaningfulAlpha) {
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
        AlphaBlend(
            GetImageHDC(),
            dstX,
            dstY,
            drawW,
            drawH,
            GetImageHDC(const_cast<IMAGE*>(&image)),
            0,
            0,
            srcW,
            srcH,
            bf);
        return;
    }

    TransparentBlt(
        GetImageHDC(),
        dstX,
        dstY,
        drawW,
        drawH,
        GetImageHDC(const_cast<IMAGE*>(&image)),
        0,
        0,
        srcW,
        srcH,
        RGB(0, 0, 0));
}

void DrawImageAutoRotated(const IMAGE& image, bool hasMeaningfulAlpha, int centerX, int centerY, int dstW, int dstH, float angleDegrees) {
    const int srcW = image.getwidth();
    const int srcH = image.getheight();
    if (srcW <= 0 || srcH <= 0 || !g_gdiplusStarted) {
        return;
    }

    Gdiplus::Bitmap* bitmap = GetCachedBitmap(image);
    if (bitmap == nullptr) {
        DrawImageAuto(image, hasMeaningfulAlpha, centerX - dstW / 2, centerY - dstH / 2, dstW, dstH);
        return;
    }

    Gdiplus::ImageAttributes attrs;
    if (!hasMeaningfulAlpha) {
        attrs.SetColorKey(Gdiplus::Color(0, 0, 0), Gdiplus::Color(0, 0, 0));
        attrs.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
    }

    DrawRotatedGdiplusImage(
        bitmap,
        centerX,
        centerY,
        dstW,
        dstH,
        angleDegrees,
        hasMeaningfulAlpha ? nullptr : &attrs);
}

bool GetImageFileVisibleBounds(const TCHAR* path, int& outX, int& outY, int& outW, int& outH) {
    outX = 0;
    outY = 0;
    outW = 0;
    outH = 0;
    if (path == nullptr || path[0] == _T('\0') || !g_gdiplusStarted) {
        return false;
    }

    const std::wstring key(path);
    const auto cached = g_fileVisibleBoundsCache.find(key);
    if (cached != g_fileVisibleBoundsCache.end()) {
        if (!cached->second.valid) {
            return false;
        }
        outX = cached->second.x;
        outY = cached->second.y;
        outW = cached->second.w;
        outH = cached->second.h;
        return true;
    }

    CachedVisibleBounds bounds;
    Gdiplus::Bitmap bitmap(path, FALSE);
    if (bitmap.GetLastStatus() != Gdiplus::Ok) {
        g_fileVisibleBoundsCache.emplace(key, bounds);
        return false;
    }

    const int width = static_cast<int>(bitmap.GetWidth());
    const int height = static_cast<int>(bitmap.GetHeight());
    if (width <= 0 || height <= 0) {
        g_fileVisibleBoundsCache.emplace(key, bounds);
        return false;
    }

    int minX = width;
    int minY = height;
    int maxX = -1;
    int maxY = -1;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Gdiplus::Color color;
            if (bitmap.GetPixel(x, y, &color) != Gdiplus::Ok || color.GetA() <= 8) {
                continue;
            }
            if (x < minX) minX = x;
            if (y < minY) minY = y;
            if (x > maxX) maxX = x;
            if (y > maxY) maxY = y;
        }
    }

    if (maxX < minX || maxY < minY) {
        g_fileVisibleBoundsCache.emplace(key, bounds);
        return false;
    }

    bounds.valid = true;
    bounds.x = minX;
    bounds.y = minY;
    bounds.w = maxX - minX + 1;
    bounds.h = maxY - minY + 1;
    outX = bounds.x;
    outY = bounds.y;
    outW = bounds.w;
    outH = bounds.h;
    g_fileVisibleBoundsCache.emplace(key, bounds);
    return true;
}

bool DrawImageFileRegion(
    const TCHAR* path,
    int srcX,
    int srcY,
    int srcW,
    int srcH,
    int dstX,
    int dstY,
    int dstW,
    int dstH) {
    if (srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0) {
        return false;
    }

    Gdiplus::Image* image = GetCachedFileImage(path);
    if (image == nullptr) {
        return false;
    }

    Gdiplus::Graphics graphics(GetImageHDC());
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

    Gdiplus::ImageAttributes attrs;
    attrs.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
    const Gdiplus::Status status = graphics.DrawImage(
        image,
        Gdiplus::Rect(dstX, dstY, dstW, dstH),
        srcX,
        srcY,
        srcW,
        srcH,
        Gdiplus::UnitPixel,
        &attrs);
    return status == Gdiplus::Ok;
}

void DrawImageFileRotated(const TCHAR* path, int centerX, int centerY, int dstW, int dstH, float angleDegrees) {
    Gdiplus::Image* image = GetCachedFileImage(path);
    if (image == nullptr) {
        return;
    }

    DrawRotatedGdiplusImage(image, centerX, centerY, dstW, dstH, angleDegrees, nullptr);
}

}  
