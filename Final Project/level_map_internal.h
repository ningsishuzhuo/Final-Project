#ifndef LEVEL_MAP_INTERNAL_H
#define LEVEL_MAP_INTERNAL_H

#include "asset_paths.h"
#include "game_data.h"
#include "globals.h"
#include "level_map_constants.h"
#include "level_map_combat_api.h"
#include "level_map_enemy_api.h"
#include "level_map_obstacle_api.h"
#include "level_map_player_status.h"
#include "level_map_render_api.h"
#include "level_map_state.h"
#include "level_map_types.h"
#include "render_utils.h"

#include <graphics.h>
#include <windows.h>

#include <vector>

namespace LevelMapInternal {

template <typename T>
void PushCapped(std::vector<T>& items, const T& item, size_t maxCount) {
    if (maxCount == 0) {
        return;
    }

    if (items.size() < maxCount) {
        items.push_back(item);
        return;
    }

    for (size_t i = 1; i < items.size(); ++i) {
        items[i - 1] = items[i];
    }
    items.back() = item;
}

}  

#endif
