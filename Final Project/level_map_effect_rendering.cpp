#include "level_map_internal.h"
#include "level_map_rendering_effects_internal.h"

namespace LevelMapInternal {

bool IsWithinLayerRange(float y, int minYInclusive, int maxYExclusive) {
    return y >= static_cast<float>(minYInclusive) && y < static_cast<float>(maxYExclusive);
}

bool IsScreenVisible(int centerX, int centerY, int radiusX, int radiusY) {
    return centerX >= -radiusX &&
        centerX <= GAME_WINDOW_WIDTH + radiusX &&
        centerY >= -radiusY &&
        centerY <= GAME_WINDOW_HEIGHT + radiusY;
}
void DrawGrid() {
    
}

}  
