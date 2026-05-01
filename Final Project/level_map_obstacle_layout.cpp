#include "level_map_internal.h"
#include "level_map_obstacle_internal.h"

#include <cmath>
#include <vector>

namespace LevelMapInternal {

void BuildObstacleLayout() {
    std::vector<MapObstacle>& obstacles = Obstacles();
    obstacles.clear();
    obstacles.reserve(300);

    const float left = static_cast<float>(g_iceRegionRect.left + 130);
    const float right = static_cast<float>(g_iceRegionRect.right - 130);
    const float top = static_cast<float>(g_iceRegionRect.top + 130);
    const float bottom = static_cast<float>(g_iceRegionRect.bottom - 130);
    const float centerX = (left + right) * 0.5f;
    const float centerY = (top + bottom) * 0.5f;

    auto hash01 = [](unsigned int key) {
        unsigned int x = key * 747796405u + 2891336453u;
        x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
        x = (x >> 22u) ^ x;
        return static_cast<float>(x & 0xFFFFu) / 65535.0f;
    };

    auto inCoreClearZone = [&](float x, float y) {
        return std::fabs(x - centerX) < 180.0f && std::fabs(y - centerY) < 150.0f;
    };

    auto inMiddleSparseZone = [&](float x, float y) {
        return std::fabs(x - centerX) < 420.0f && std::fabs(y - centerY) < 340.0f;
    };

    auto inMainCorridor = [&](float x, float y) {
        return std::fabs(x - centerX) < 130.0f || std::fabs(y - centerY) < 130.0f;
    };

    auto canPlaceSparse = [&](float x, float y, float minDistance) {
        const float minDistanceSq = minDistance * minDistance;
        for (const MapObstacle& obstacle : obstacles) {
            const float dx = obstacle.x - x;
            const float dy = obstacle.y - y;
            if (dx * dx + dy * dy < minDistanceSq) {
                return false;
            }
        }
        return true;
    };

    auto addPatch = [&](
        float originX,
        float originY,
        int rows,
        int cols,
        MapObstacleKind kind,
        int width,
        int height,
        bool destructible,
        int seed) {
        int useRows = rows > 3 ? 3 : rows;
        int useCols = cols > 3 ? 3 : cols;
        const bool isWallPatch = IsWallKind(kind);
        const bool isWallOrCratePatch = IsWallOrCrateKind(kind);

        const float stepX = isWallOrCratePatch ? static_cast<float>(width) : static_cast<float>(width) * 0.82f;
        const float stepY = isWallOrCratePatch ? static_cast<float>(height) : static_cast<float>(height) * 0.79f;
        const float centerCol = static_cast<float>(useCols - 1) * 0.5f;
        const float centerRow = static_cast<float>(useRows - 1) * 0.5f;
        for (int r = 0; r < useRows; ++r) {
            for (int c = 0; c < useCols; ++c) {
                const unsigned int key = static_cast<unsigned int>(seed + r * 73 + c * 97 + r * c * 37);
                const float holeThreshold = isWallOrCratePatch ? 0.0f : (isWallPatch ? 0.34f : 0.18f);
                if (hash01(key) < holeThreshold) {
                    continue;
                }
                const float rowOffset = isWallOrCratePatch ? 0.0f : ((r & 1) == 0 ? -1.0f : 1.0f) * stepX * 0.12f;
                const float skew = isWallOrCratePatch ? 0.0f : (static_cast<float>(r) - centerRow) * (hash01(key + 7u) - 0.5f) * 16.0f;
                const float jitterX = isWallOrCratePatch ? 0.0f : (hash01(key + 11u) - 0.5f) * 9.0f;
                const float jitterY = isWallOrCratePatch ? 0.0f : (hash01(key + 19u) - 0.5f) * 8.0f;
                float x = originX + (static_cast<float>(c) - centerCol) * stepX + rowOffset + skew + jitterX;
                float y = originY + (static_cast<float>(r) - centerRow) * stepY + jitterY;
                if (inCoreClearZone(x, y)) {
                    continue;
                }
                if (inMainCorridor(x, y)) {
                    continue;
                }
                if (!isWallOrCratePatch && inMiddleSparseZone(x, y) && hash01(key + 31u) < 0.42f) {
                    continue;
                }
                if (x < left + 40.0f || x > right - 40.0f || y < top + 40.0f || y > bottom - 40.0f) {
                    continue;
                }
                float minPatchDistance = inMiddleSparseZone(x, y) ? 90.0f : 66.0f;
                if (isWallPatch) {
                    minPatchDistance += 16.0f;
                }
                if (isWallOrCratePatch) {
                    minPatchDistance = static_cast<float>(MinDimension(width, height)) * 0.96f;
                }
                if (!canPlaceSparse(x, y, minPatchDistance)) {
                    continue;
                }
                AddObstacle(kind, x, y, width, height, destructible);
            }
        }
    };

    struct PatchSeed {
        float dx;
        float dy;
        int rows;
        int cols;
        MapObstacleKind kind;
        int width;
        int height;
        bool destructible;
        int seed;
    };

    const PatchSeed wallPatches[] = {
        { -760.0f, -500.0f, 2, 3, MapObstacleKind::IceWall, kWallAndCrateSize, kWallAndCrateSize, false, 101 },
        { 740.0f, -480.0f, 2, 3, MapObstacleKind::OreWall, kWallAndCrateSize, kWallAndCrateSize, false, 167 },
        { -860.0f, 130.0f, 2, 2, MapObstacleKind::OreWall, kWallAndCrateSize, kWallAndCrateSize, false, 223 },
        { 860.0f, 170.0f, 2, 2, MapObstacleKind::IceWall, kWallAndCrateSize, kWallAndCrateSize, false, 289 },
        { -520.0f, 600.0f, 2, 3, MapObstacleKind::IceWall, kWallAndCrateSize, kWallAndCrateSize, false, 337 },
        { 520.0f, 600.0f, 2, 3, MapObstacleKind::OreWall, kWallAndCrateSize, kWallAndCrateSize, false, 401 },
    };
    for (const PatchSeed& patch : wallPatches) {
        addPatch(centerX + patch.dx, centerY + patch.dy, patch.rows, patch.cols, patch.kind, patch.width, patch.height, patch.destructible, patch.seed);
    }

    const PatchSeed cratePatches[] = {
        { -740.0f, -220.0f, 3, 3, MapObstacleKind::Crate, kWallAndCrateSize, kWallAndCrateSize, true, 823 },
        { 740.0f, -220.0f, 3, 3, MapObstacleKind::Crate, kWallAndCrateSize, kWallAndCrateSize, true, 859 },
        { -760.0f, 260.0f, 3, 3, MapObstacleKind::Crate, kWallAndCrateSize, kWallAndCrateSize, true, 887 },
        { 760.0f, 260.0f, 3, 3, MapObstacleKind::Crate, kWallAndCrateSize, kWallAndCrateSize, true, 919 },
        { -360.0f, 620.0f, 3, 3, MapObstacleKind::Crate, kWallAndCrateSize, kWallAndCrateSize, true, 953 },
        { 360.0f, -620.0f, 3, 3, MapObstacleKind::Crate, kWallAndCrateSize, kWallAndCrateSize, true, 991 },
    };
    for (const PatchSeed& patch : cratePatches) {
        addPatch(centerX + patch.dx, centerY + patch.dy, patch.rows, patch.cols, patch.kind, patch.width, patch.height, patch.destructible, patch.seed);
    }

    auto addMixedObstacle = [&](MapObstacleKind kind, float x, float y, unsigned int key) {
        int width = 86;
        int height = 82;
        bool destructible = false;
        float minDist = 86.0f;
        switch (kind) {
        case MapObstacleKind::Rock:
            width = 90;
            height = 86;
            minDist = 98.0f;
            break;
        case MapObstacleKind::Minecart:
            width = 92;
            height = 80;
            minDist = 98.0f;
            break;
        case MapObstacleKind::BrokenMinecart:
            width = 88;
            height = 76;
            minDist = 94.0f;
            break;
        case MapObstacleKind::OreWall:
            width = kWallAndCrateSize;
            height = kWallAndCrateSize;
            minDist = 96.0f;
            break;
        case MapObstacleKind::IceWall:
            width = kWallAndCrateSize;
            height = kWallAndCrateSize;
            minDist = 96.0f;
            break;
        case MapObstacleKind::Rail:
            width = kRailSize;
            height = kRailSize;
            minDist = 86.0f;
            break;
        case MapObstacleKind::Crate:
            width = kWallAndCrateSize;
            height = kWallAndCrateSize;
            destructible = true;
            minDist = 96.0f;
            break;
        case MapObstacleKind::Torch:
            width = 60;
            height = 60;
            minDist = 66.0f;
            break;
        }

        const bool isWallOrCrate = IsWallOrCrateKind(kind);
        if (!isWallOrCrate) {
            x += (hash01(key + 1u) - 0.5f) * 16.0f;
            y += (hash01(key + 9u) - 0.5f) * 15.0f;
        } else {
            x = left + std::round((x - left) / kGridSnapForWallOrCrate) * kGridSnapForWallOrCrate;
            y = top + std::round((y - top) / kGridSnapForWallOrCrate) * kGridSnapForWallOrCrate;
        }
        if (inCoreClearZone(x, y)) {
            return;
        }
        if (x < left + 28.0f || x > right - 28.0f || y < top + 28.0f || y > bottom - 28.0f) {
            return;
        }
        if (inMainCorridor(x, y)) {
            return;
        }
        if (inMiddleSparseZone(x, y) && hash01(key + 21u) < 0.40f) {
            return;
        }
        if (inMiddleSparseZone(x, y)) {
            minDist += 12.0f;
        }
        if (isWallOrCrate) {
            minDist = static_cast<float>(MinDimension(width, height));
        }
        if (!canPlaceSparse(x, y, minDist)) {
            return;
        }

        
        if (kind == MapObstacleKind::Minecart ||
            kind == MapObstacleKind::BrokenMinecart ||
            kind == MapObstacleKind::Rock ||
            kind == MapObstacleKind::Rail) {
            const float keepClearDistSq = 180.0f * 180.0f;
            for (const MapObstacle& existing : obstacles) {
                if (existing.kind != MapObstacleKind::IceWall &&
                    existing.kind != MapObstacleKind::OreWall &&
                    existing.kind != MapObstacleKind::Crate) {
                    continue;
                }
                const float dx = existing.x - x;
                const float dy = existing.y - y;
                if (dx * dx + dy * dy < keepClearDistSq) {
                    return;
                }
            }
        }

        AddObstacle(kind, x, y, width, height, destructible);
    };

    struct MixedSeed {
        float dx;
        float dy;
        int count;
        unsigned int seed;
    };

    const MixedSeed mixedClusters[] = {
        { -900.0f, -320.0f, 4, 911u },
        { -900.0f, 120.0f, 4, 977u },
        { -620.0f, 560.0f, 4, 1031u },
        { -420.0f, -640.0f, 3, 1109u },
        { 420.0f, -640.0f, 3, 1181u },
        { 620.0f, 560.0f, 4, 1259u },
        { 900.0f, -260.0f, 4, 1327u },
        { 900.0f, 180.0f, 4, 1399u },
        { -260.0f, -260.0f, 3, 1453u },
        { 260.0f, -240.0f, 3, 1511u },
        { -230.0f, 260.0f, 3, 1583u },
        { 240.0f, 240.0f, 3, 1667u },
    };

    for (const MixedSeed& cluster : mixedClusters) {
        const float baseX = centerX + cluster.dx;
        const float baseY = centerY + cluster.dy;
        for (int i = 0; i < cluster.count; ++i) {
            const unsigned int key = cluster.seed + static_cast<unsigned int>(i * 131);
            const float angle = hash01(key + 3u) * 6.28318f;
            const float radius = 26.0f + hash01(key + 5u) * 168.0f;
            const float x = baseX + std::cos(angle) * radius;
            const float y = baseY + std::sin(angle) * radius * (0.82f + hash01(key + 7u) * 0.46f);

            MapObstacleKind kind = MapObstacleKind::Rock;
            const float selector = hash01(key + 11u);
            if (selector < 0.12f) {
                kind = MapObstacleKind::Torch;
            } else if (selector < 0.34f) {
                kind = MapObstacleKind::Minecart;
            } else if (selector < 0.46f) {
                kind = MapObstacleKind::BrokenMinecart;
            } else if (selector < 0.68f) {
                kind = MapObstacleKind::Rail;
            } else {
                kind = MapObstacleKind::Rock;
            }
            addMixedObstacle(kind, x, y, key);
        }
    }

    for (int i = 0; i < 12; ++i) {
        const unsigned int key = 1601u + static_cast<unsigned int>(i * 83);
        const float x = left + (right - left) * hash01(key + 1u);
        const float y = top + (bottom - top) * hash01(key + 2u);
        if (inMainCorridor(x, y)) {
            continue;
        }
        if (inMiddleSparseZone(x, y)) {
            continue;
        }
        const float selector = hash01(key + 3u);

        MapObstacleKind kind = MapObstacleKind::Rock;
        if (selector < 0.16f) {
            kind = MapObstacleKind::Rail;
        } else if (selector < 0.32f) {
            kind = MapObstacleKind::Torch;
        } else if (selector < 0.56f) {
            kind = MapObstacleKind::Minecart;
        } else if (selector < 0.68f) {
            kind = MapObstacleKind::BrokenMinecart;
        } else {
            kind = MapObstacleKind::Rock;
        }
        addMixedObstacle(kind, x, y, key);
    }
}

}  
