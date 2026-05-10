#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include <graphics.h>
#include <tchar.h>
#include <windows.h>
#include <gdiplus.h>

#include <memory>
#include <vector>

namespace RenderUtils {

struct AnimatedGif {
    std::unique_ptr<Gdiplus::Image> image;
    std::vector<UINT> delaysMs;
    GUID dimensionGuid = {};
    UINT frameCount = 0;
    UINT currentFrame = 0;
    ULONGLONG nextTick = 0;

    void Reset();
    bool IsLoaded() const;
};

bool HasImage(const IMAGE& image);
ULONGLONG NowTickMs();
bool HasMeaningfulAlpha(const IMAGE& image);
bool IsPointInRect(int x, int y, const RECT& rect);

void AcquireGdiplus();
void ReleaseGdiplus();
bool IsGdiplusReady();

void LoadImageFlexible(IMAGE& dst, const TCHAR* path, int targetW, int targetH);
void LoadAnimatedGif(AnimatedGif& gif, const TCHAR* path);
void UpdateAnimatedGifFrame(AnimatedGif& gif);

void DrawImageAuto(const IMAGE& image, bool hasMeaningfulAlpha, int dstX, int dstY, int dstW, int dstH);
void DrawImageAutoRotated(const IMAGE& image, bool hasMeaningfulAlpha, int centerX, int centerY, int dstW, int dstH, float angleDegrees);
bool GetImageFileVisibleBounds(const TCHAR* path, int& outX, int& outY, int& outW, int& outH);
bool DrawImageFileRegion(const TCHAR* path, int srcX, int srcY, int srcW, int srcH, int dstX, int dstY, int dstW, int dstH);
void DrawImageFileRotated(const TCHAR* path, int centerX, int centerY, int dstW, int dstH, float angleDegrees);
void DrawAnimatedGif(const AnimatedGif& gif, int x, int y, int w, int h, bool flipHorizontal = false);
void DrawAnimatedGifOpacity(const AnimatedGif& gif, int x, int y, int w, int h, float opacity, bool flipHorizontal = false);

}  

#endif
