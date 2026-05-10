#include "level_map_internal.h"
#include "level_map_combat_internal.h"
#include "level_map_obstacle_internal.h"

#include <cmath>

namespace LevelMapInternal {
namespace {

float ClampFloat(float v, float low, float high) {
    if (v < low) return low;
    if (v > high) return high;
    return v;
}

struct ObstacleAabb {
    float minX = 0.0f;
    float maxX = 0.0f;
    float minY = 0.0f;
    float maxY = 0.0f;
};

enum class PushDirection {
    Left,
    Right,
    Top,
    Bottom
};

struct ResolvedCirclePosition {
    float x = 0.0f;
    float y = 0.0f;
    bool blocked = false;
};

ObstacleAabb GetObstacleAabb(const MapObstacle& obstacle, float collisionScale) {
    const float halfW = static_cast<float>(obstacle.width) * 0.5f * collisionScale;
    const float halfH = static_cast<float>(obstacle.height) * 0.5f * collisionScale;
    return ObstacleAabb{
        obstacle.x - halfW,
        obstacle.x + halfW,
        obstacle.y - halfH,
        obstacle.y + halfH
    };
}

bool IsObstacleBlocking(const MapObstacle& obstacle) {
    if (!obstacle.solid) {
        return false;
    }
    if (obstacle.destructible && obstacle.destroyed) {
        return false;
    }
    return true;
}

float GetObstacleCollisionScale(MapObstacleKind kind) {
    switch (kind) {
    case MapObstacleKind::IceWall:
    case MapObstacleKind::OreWall:
    case MapObstacleKind::Crate:
        return 0.92f;
    case MapObstacleKind::Rock:
        return 0.70f;
    case MapObstacleKind::Minecart:
        return 0.58f;
    case MapObstacleKind::BrokenMinecart:
        return 0.56f;
    case MapObstacleKind::Rail:
        return 0.56f;
    case MapObstacleKind::Torch:
        return 0.60f;
    default:
        return 1.0f;
    }
}

bool CircleIntersectsObstacle(const MapObstacle& obstacle, float x, float y, float radius) {
    if (!IsObstacleBlocking(obstacle)) {
        return false;
    }

    const float collisionScale = GetObstacleCollisionScale(obstacle.kind);
    const ObstacleAabb bounds = GetObstacleAabb(obstacle, collisionScale);
    const float nearestX = ClampFloat(x, bounds.minX, bounds.maxX);
    const float nearestY = ClampFloat(y, bounds.minY, bounds.maxY);
    const float dx = x - nearestX;
    const float dy = y - nearestY;
    return dx * dx + dy * dy <= radius * radius;
}

bool PointIntersectsObstacle(const MapObstacle& obstacle, float x, float y) {
    if (!IsObstacleBlocking(obstacle)) {
        return false;
    }

    const float collisionScale = GetObstacleCollisionScale(obstacle.kind);
    const ObstacleAabb bounds = GetObstacleAabb(obstacle, collisionScale);
    return x >= bounds.minX &&
        x <= bounds.maxX &&
        y >= bounds.minY &&
        y <= bounds.maxY;
}

void PushOutFromAabb(float& x, float& y, const ObstacleAabb& bounds, float offset) {
    // 处理圆心落在盒内时没有推出梯度的情况。
    const float distanceToLeft = std::fabs(x - bounds.minX);
    const float distanceToRight = std::fabs(bounds.maxX - x);
    const float distanceToTop = std::fabs(y - bounds.minY);
    const float distanceToBottom = std::fabs(bounds.maxY - y);

    float minDistance = distanceToLeft;
    PushDirection direction = PushDirection::Left;

    if (distanceToRight < minDistance) {
        minDistance = distanceToRight;
        direction = PushDirection::Right;
    }
    if (distanceToTop < minDistance) {
        minDistance = distanceToTop;
        direction = PushDirection::Top;
    }
    if (distanceToBottom < minDistance) {
        direction = PushDirection::Bottom;
    }

    switch (direction) {
    case PushDirection::Left:
        x = bounds.minX - offset;
        break;
    case PushDirection::Right:
        x = bounds.maxX + offset;
        break;
    case PushDirection::Top:
        y = bounds.minY - offset;
        break;
    case PushDirection::Bottom:
        y = bounds.maxY + offset;
        break;
    }
}

bool IsBreakableCrate(const MapObstacle& obstacle) {
    return obstacle.kind == MapObstacleKind::Crate &&
        obstacle.destructible &&
        !obstacle.destroyed;
}

void DamageBreakableObstacle(MapObstacle& obstacle) {
    if (!obstacle.destructible || obstacle.destroyed) {
        return;
    }

    --obstacle.hp;
    if (obstacle.hp <= 0) {
        obstacle.destroyed = true;
        SpawnDropsOnCrateDestroyed(static_cast<int>(obstacle.x), static_cast<int>(obstacle.y));
    }
}

ResolvedCirclePosition ResolveCircleMovementAgainstObstacles(
    float currentX,
    float currentY,
    float previousX,
    float previousY,
    float radius) {
    bool blocked = false;
    for (const MapObstacle& obstacle : Obstacles()) {
        if (CircleIntersectsObstacle(obstacle, currentX, currentY, radius)) {
            blocked = true;
            break;
        }
    }
    if (!blocked) {
        return { currentX, currentY, false };
    }

    // 完整位移受阻时尝试沿单轴滑动。
    bool blockXOnly = false;
    bool blockYOnly = false;
    for (const MapObstacle& obstacle : Obstacles()) {
        if (CircleIntersectsObstacle(obstacle, currentX, previousY, radius)) {
            blockXOnly = true;
        }
        if (CircleIntersectsObstacle(obstacle, previousX, currentY, radius)) {
            blockYOnly = true;
        }
    }

    if (!blockXOnly) {
        return { currentX, previousY, true };
    }
    if (!blockYOnly) {
        return { previousX, currentY, true };
    }

    return { previousX, previousY, true };
}

}

void ResolvePlayerPositionAgainstObstacles(int previousX, int previousY) {
    constexpr float kPlayerObstacleRadius = 28.0f;
    const ResolvedCirclePosition resolved = ResolveCircleMovementAgainstObstacles(
        static_cast<float>(g_playerX),
        static_cast<float>(g_playerY),
        static_cast<float>(previousX),
        static_cast<float>(previousY),
        kPlayerObstacleRadius);
    if (!resolved.blocked) {
        return;
    }

    g_playerX = static_cast<int>(resolved.x);
    g_playerY = static_cast<int>(resolved.y);
}

void ResolveEnemyPositionAgainstObstacles(float previousX, float previousY, float radius) {
    const ResolvedCirclePosition resolved = ResolveCircleMovementAgainstObstacles(
        g_enemy.exactX,
        g_enemy.exactY,
        previousX,
        previousY,
        radius);
    if (!resolved.blocked) {
        return;
    }

    g_enemy.exactX = resolved.x;
    g_enemy.exactY = resolved.y;
    RefreshEnemyGridPosition();
}

bool HandleProjectileObstacleHit(Projectile& projectile) {
    for (MapObstacle& obstacle : Obstacles()) {
        if (!PointIntersectsObstacle(obstacle, projectile.x, projectile.y)) {
            continue;
        }

        projectile.active = false;
        DamageBreakableObstacle(obstacle);
        return true;
    }
    return false;
}

void ApplyMeleeObstacleHitInFrontArc(float originX, float originY, float dirX, float dirY, float range) {
    if (range <= 0.0f) {
        return;
    }

    for (MapObstacle& obstacle : Obstacles()) {
        if (!IsBreakableCrate(obstacle)) {
            continue;
        }

        const float relX = obstacle.x - originX;
        const float relY = obstacle.y - originY;
        if (!IsPointInFrontArc(relX, relY, dirX, dirY, range)) {
            continue;
        }

        DamageBreakableObstacle(obstacle);
    }
}

void ApplyCircleObstacleHit(float centerX, float centerY, float radius) {
    if (radius <= 0.0f) {
        return;
    }

    const float radiusSq = radius * radius;
    for (MapObstacle& obstacle : Obstacles()) {
        if (!IsBreakableCrate(obstacle)) {
            continue;
        }

        const float relX = obstacle.x - centerX;
        const float relY = obstacle.y - centerY;
        if (relX * relX + relY * relY > radiusSq) {
            continue;
        }

        DamageBreakableObstacle(obstacle);
    }
}

bool IsCircleBlockedByObstacles(float x, float y, float radius) {
    for (const MapObstacle& obstacle : Obstacles()) {
        if (CircleIntersectsObstacle(obstacle, x, y, radius)) {
            return true;
        }
    }
    return false;
}

void ResolveCircleOutsideObstacles(float& x, float& y, float radius) {
    constexpr float kPushOutEpsilon = 1.0f;
    constexpr int kMaxIterations = 10;
    const float effectiveRadius = (radius > 0.0f) ? radius : 0.0f;

    for (int iteration = 0; iteration < kMaxIterations; ++iteration) {
        const MapObstacle* hitObstacle = nullptr;
        ObstacleAabb hitBounds;
        for (const MapObstacle& obstacle : Obstacles()) {
            if (!CircleIntersectsObstacle(obstacle, x, y, effectiveRadius)) {
                continue;
            }
            hitObstacle = &obstacle;
            hitBounds = GetObstacleAabb(obstacle, GetObstacleCollisionScale(obstacle.kind));
            break;
        }

        if (hitObstacle == nullptr) {
            return;
        }

        const float nearestX = ClampFloat(x, hitBounds.minX, hitBounds.maxX);
        const float nearestY = ClampFloat(y, hitBounds.minY, hitBounds.maxY);
        const float pushX = x - nearestX;
        const float pushY = y - nearestY;
        const float distSquared = pushX * pushX + pushY * pushY;
        if (distSquared > 0.0001f) {
            const float distance = std::sqrt(distSquared);
            const float overlap = effectiveRadius - distance + kPushOutEpsilon;
            if (overlap > 0.0f) {
                const float invDistance = 1.0f / distance;
                x += pushX * invDistance * overlap;
                y += pushY * invDistance * overlap;
            }
            continue;
        }

        PushOutFromAabb(x, y, hitBounds, effectiveRadius + kPushOutEpsilon);
    }
}

void ResolvePointOutsideObstacles(float& x, float& y) {
    constexpr float kPushOutEpsilon = 1.0f;
    constexpr int kMaxIterations = 8;

    for (int iteration = 0; iteration < kMaxIterations; ++iteration) {
        const MapObstacle* hitObstacle = nullptr;
        ObstacleAabb hitBounds;
        for (const MapObstacle& obstacle : Obstacles()) {
            if (!PointIntersectsObstacle(obstacle, x, y)) {
                continue;
            }
            hitObstacle = &obstacle;
            hitBounds = GetObstacleAabb(obstacle, GetObstacleCollisionScale(obstacle.kind));
            break;
        }

        if (hitObstacle == nullptr) {
            return;
        }

        PushOutFromAabb(x, y, hitBounds, kPushOutEpsilon);
    }
}

}
