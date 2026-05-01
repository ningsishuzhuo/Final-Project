#include "level_map_internal.h"
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

bool IsPointInFrontArc(float relX, float relY, float dirX, float dirY, float range) {
    const float rangeSq = range * range;
    if (relX * relX + relY * relY > rangeSq) {
        return false;
    }
    return relX * dirX + relY * dirY >= 0.0f;
}

}  

void ResolvePlayerPositionAgainstObstacles(int previousX, int previousY) {
    constexpr float kPlayerObstacleRadius = 28.0f;
    bool blocked = false;
    for (const MapObstacle& obstacle : Obstacles()) {
        if (CircleIntersectsObstacle(obstacle, static_cast<float>(g_playerX), static_cast<float>(g_playerY), kPlayerObstacleRadius)) {
            blocked = true;
            break;
        }
    }
    if (!blocked) {
        return;
    }

    const int candidateX = g_playerX;
    const int candidateY = g_playerY;
    bool blockXOnly = false;
    bool blockYOnly = false;
    for (const MapObstacle& obstacle : Obstacles()) {
        if (CircleIntersectsObstacle(obstacle, static_cast<float>(candidateX), static_cast<float>(previousY), kPlayerObstacleRadius)) {
            blockXOnly = true;
        }
        if (CircleIntersectsObstacle(obstacle, static_cast<float>(previousX), static_cast<float>(candidateY), kPlayerObstacleRadius)) {
            blockYOnly = true;
        }
    }

    if (!blockXOnly) {
        g_playerY = previousY;
        return;
    }
    if (!blockYOnly) {
        g_playerX = previousX;
        return;
    }

    g_playerX = previousX;
    g_playerY = previousY;
}

void ResolveEnemyPositionAgainstObstacles(float previousX, float previousY, float radius) {
    bool blocked = false;
    for (const MapObstacle& obstacle : Obstacles()) {
        if (CircleIntersectsObstacle(obstacle, g_enemy.exactX, g_enemy.exactY, radius)) {
            blocked = true;
            break;
        }
    }
    if (!blocked) {
        return;
    }

    const float candidateX = g_enemy.exactX;
    const float candidateY = g_enemy.exactY;
    bool blockXOnly = false;
    bool blockYOnly = false;
    for (const MapObstacle& obstacle : Obstacles()) {
        if (CircleIntersectsObstacle(obstacle, candidateX, previousY, radius)) {
            blockXOnly = true;
        }
        if (CircleIntersectsObstacle(obstacle, previousX, candidateY, radius)) {
            blockYOnly = true;
        }
    }

    if (!blockXOnly) {
        g_enemy.exactY = previousY;
        RefreshEnemyGridPosition();
        return;
    }
    if (!blockYOnly) {
        g_enemy.exactX = previousX;
        RefreshEnemyGridPosition();
        return;
    }

    g_enemy.exactX = previousX;
    g_enemy.exactY = previousY;
    RefreshEnemyGridPosition();
}

bool HandleProjectileObstacleHit(Projectile& projectile) {
    for (MapObstacle& obstacle : Obstacles()) {
        if (!PointIntersectsObstacle(obstacle, projectile.x, projectile.y)) {
            continue;
        }

        projectile.active = false;
        if (obstacle.destructible && !obstacle.destroyed) {
            --obstacle.hp;
            if (obstacle.hp <= 0) {
                obstacle.destroyed = true;
                SpawnDropsOnCrateDestroyed(static_cast<int>(obstacle.x), static_cast<int>(obstacle.y));
            }
        }
        return true;
    }
    return false;
}

void ApplyMeleeObstacleHitInFrontArc(float originX, float originY, float dirX, float dirY, float range) {
    if (range <= 0.0f) {
        return;
    }

    for (MapObstacle& obstacle : Obstacles()) {
        if (obstacle.kind != MapObstacleKind::Crate || !obstacle.destructible || obstacle.destroyed) {
            continue;
        }

        const float relX = obstacle.x - originX;
        const float relY = obstacle.y - originY;
        if (!IsPointInFrontArc(relX, relY, dirX, dirY, range)) {
            continue;
        }

        --obstacle.hp;
        if (obstacle.hp <= 0) {
            obstacle.destroyed = true;
            SpawnDropsOnCrateDestroyed(static_cast<int>(obstacle.x), static_cast<int>(obstacle.y));
        }
    }
}

void ApplyCircleObstacleHit(float centerX, float centerY, float radius) {
    if (radius <= 0.0f) {
        return;
    }

    const float radiusSq = radius * radius;
    for (MapObstacle& obstacle : Obstacles()) {
        if (obstacle.kind != MapObstacleKind::Crate || !obstacle.destructible || obstacle.destroyed) {
            continue;
        }

        const float relX = obstacle.x - centerX;
        const float relY = obstacle.y - centerY;
        if (relX * relX + relY * relY > radiusSq) {
            continue;
        }

        --obstacle.hp;
        if (obstacle.hp <= 0) {
            obstacle.destroyed = true;
            SpawnDropsOnCrateDestroyed(static_cast<int>(obstacle.x), static_cast<int>(obstacle.y));
        }
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

        const float distanceToLeft = std::fabs(x - hitBounds.minX);
        const float distanceToRight = std::fabs(hitBounds.maxX - x);
        const float distanceToTop = std::fabs(y - hitBounds.minY);
        const float distanceToBottom = std::fabs(hitBounds.maxY - y);

        float minDistance = distanceToLeft;
        enum class PushDirection { Left, Right, Top, Bottom };
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

        const float offset = effectiveRadius + kPushOutEpsilon;
        switch (direction) {
        case PushDirection::Left:
            x = hitBounds.minX - offset;
            break;
        case PushDirection::Right:
            x = hitBounds.maxX + offset;
            break;
        case PushDirection::Top:
            y = hitBounds.minY - offset;
            break;
        case PushDirection::Bottom:
            y = hitBounds.maxY + offset;
            break;
        }
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

        const float distanceToLeft = std::fabs(x - hitBounds.minX);
        const float distanceToRight = std::fabs(hitBounds.maxX - x);
        const float distanceToTop = std::fabs(y - hitBounds.minY);
        const float distanceToBottom = std::fabs(hitBounds.maxY - y);

        float minDistance = distanceToLeft;
        enum class PushDirection { Left, Right, Top, Bottom };
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
            x = hitBounds.minX - kPushOutEpsilon;
            break;
        case PushDirection::Right:
            x = hitBounds.maxX + kPushOutEpsilon;
            break;
        case PushDirection::Top:
            y = hitBounds.minY - kPushOutEpsilon;
            break;
        case PushDirection::Bottom:
            y = hitBounds.maxY + kPushOutEpsilon;
            break;
        }
    }
}

}  
