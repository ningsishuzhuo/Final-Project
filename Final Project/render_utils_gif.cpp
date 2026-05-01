#include "render_utils.h"

#include <vector>

namespace RenderUtils {

void AnimatedGif::Reset() {
    image.reset();
    delaysMs.clear();
    ZeroMemory(&dimensionGuid, sizeof(dimensionGuid));
    frameCount = 0;
    currentFrame = 0;
    nextTick = 0;
}

bool AnimatedGif::IsLoaded() const {
    return image != nullptr && image->GetLastStatus() == Gdiplus::Ok;
}

void LoadAnimatedGif(AnimatedGif& gif, const TCHAR* path) {
    gif.Reset();
    if (!IsGdiplusReady() || path == nullptr) {
        return;
    }

    gif.image.reset(Gdiplus::Image::FromFile(path, FALSE));
    if (!gif.IsLoaded()) {
        gif.Reset();
        return;
    }

    UINT dimensionsCount = gif.image->GetFrameDimensionsCount();
    if (dimensionsCount == 0) {
        gif.frameCount = 1;
        gif.delaysMs.push_back(100);
        gif.nextTick = NowTickMs() + 100ULL;
        return;
    }

    std::vector<GUID> dimensions(dimensionsCount);
    gif.image->GetFrameDimensionsList(dimensions.data(), dimensionsCount);
    gif.dimensionGuid = dimensions[0];

    gif.frameCount = gif.image->GetFrameCount(&gif.dimensionGuid);
    if (gif.frameCount == 0) {
        gif.frameCount = 1;
    }

    gif.delaysMs.assign(gif.frameCount, 100);

    const UINT propSize = gif.image->GetPropertyItemSize(PropertyTagFrameDelay);
    if (propSize > 0) {
        std::vector<BYTE> propBuffer(propSize);
        Gdiplus::PropertyItem* prop = reinterpret_cast<Gdiplus::PropertyItem*>(propBuffer.data());
        if (gif.image->GetPropertyItem(PropertyTagFrameDelay, propSize, prop) == Gdiplus::Ok && prop->length >= sizeof(UINT)) {
            const UINT* delays = reinterpret_cast<const UINT*>(prop->value);
            const UINT delayCount = prop->length / sizeof(UINT);
            for (UINT i = 0; i < gif.frameCount; ++i) {
                const UINT raw = (i < delayCount) ? delays[i] : delays[delayCount - 1];
                const UINT ms = (raw > 0 ? raw : 8) * 10;
                gif.delaysMs[i] = ms < 40 ? 40 : ms;
            }
        }
    }

    gif.currentFrame = 0;
    gif.image->SelectActiveFrame(&gif.dimensionGuid, 0);
    gif.nextTick = NowTickMs() + static_cast<ULONGLONG>(gif.delaysMs[0]);
}

void UpdateAnimatedGifFrame(AnimatedGif& gif) {
    if (!gif.IsLoaded() || gif.frameCount <= 1) {
        return;
    }

    const ULONGLONG now = NowTickMs();
    int guard = 0;
    while (now >= gif.nextTick && guard < 8) {
        gif.currentFrame = (gif.currentFrame + 1) % gif.frameCount;
        gif.image->SelectActiveFrame(&gif.dimensionGuid, gif.currentFrame);
        gif.nextTick += static_cast<ULONGLONG>(gif.delaysMs[gif.currentFrame]);
        ++guard;
    }

    if (now >= gif.nextTick) {
        gif.nextTick = now + static_cast<ULONGLONG>(gif.delaysMs[gif.currentFrame]);
    }
}

void DrawAnimatedGif(const AnimatedGif& gif, int x, int y, int w, int h, bool flipHorizontal) {
    DrawAnimatedGifOpacity(gif, x, y, w, h, 1.0f, flipHorizontal);
}

void DrawAnimatedGifOpacity(const AnimatedGif& gif, int x, int y, int w, int h, float opacity, bool flipHorizontal) {
    if (!gif.IsLoaded() || !IsGdiplusReady()) {
        return;
    }

    if (opacity <= 0.0f) {
        return;
    }

    if (opacity > 1.0f) {
        opacity = 1.0f;
    }

    Gdiplus::Graphics graphics(GetImageHDC());
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    Gdiplus::ImageAttributes attrs;
    Gdiplus::ColorMatrix matrix = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, opacity, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };
    attrs.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

    if (!flipHorizontal) {
        graphics.DrawImage(
            gif.image.get(),
            Gdiplus::Rect(x, y, w, h),
            0,
            0,
            gif.image->GetWidth(),
            gif.image->GetHeight(),
            Gdiplus::UnitPixel,
            &attrs);
        return;
    }

    Gdiplus::Point destPoints[3] = {
        Gdiplus::Point(x + w, y),
        Gdiplus::Point(x, y),
        Gdiplus::Point(x + w, y + h)
    };
    graphics.DrawImage(
        gif.image.get(),
        destPoints,
        3,
        0,
        0,
        gif.image->GetWidth(),
        gif.image->GetHeight(),
        Gdiplus::UnitPixel,
        &attrs);
}

}  
