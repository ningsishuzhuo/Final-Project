#ifndef ANIMATION_H
#define ANIMATION_H

#include <graphics.h>

class LogoAnimation {
public:
    static void showStudioLogo();//播放完整动画

private:
    static void loadImages();//加载图片资源
    static void playAnimation();//执行动画播放
    static void freeImages();//释放图片内存
    static IMAGE logo_images[3];//存储logo图片
};

#endif#pragma once
