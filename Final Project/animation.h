#ifndef ANIMATION_H
#define ANIMATION_H

#include <graphics.h>

class LogoAnimation {
public:
    static void showStudioLogo();

private:
    static void loadImages();
    static void playAnimation();
    static void freeImages();
    static IMAGE logo_images[3];
};

#endif

