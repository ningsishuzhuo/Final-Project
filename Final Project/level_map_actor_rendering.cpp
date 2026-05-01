#include "level_map_internal.h"
#include "level_map_enemy_visibility.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <climits>
#include <vector>

namespace LevelMapInternal {
namespace {

struct CachedEnemyVisual {
    bool loaded = false;
    RenderUtils::AnimatedGif idleGif;
    RenderUtils::AnimatedGif walkGif;
    RenderUtils::AnimatedGif breakGif;
    IMAGE deathImage;
    bool deathHasAlpha = false;
    int referenceWidth = 0;
    int referenceHeight = 0;
};

constexpr int kEnemyKindCount = 3;

int EnemyKindIndex(GameData::EnemyKind kind) {
    const int index = static_cast<int>(kind);
    return (index >= 0 && index < kEnemyKindCount) ? index : 0;
}

int EnemyFootOffsetFromDefinition(const GameData::EnemyDefinition& def) {
    const int derived = (def.drawHeight * 3) / 8;
    return (derived > 0) ? derived : kEnemyFootOffsetY;
}

void ComputeEnemyDeathDrawSize(
    const GameData::EnemyDefinition& def,
    const IMAGE& deathImage,
    int referenceSourceW,
    int referenceSourceH,
    int& outDrawW,
    int& outDrawH) {
    outDrawW = def.drawWidth;
    outDrawH = def.drawHeight;
    const int deathSourceW = deathImage.getwidth();
    const int deathSourceH = deathImage.getheight();
    if (deathSourceW <= 0 || deathSourceH <= 0 || referenceSourceW <= 0 || referenceSourceH <= 0) {
        return;
    }

    outDrawW = def.drawWidth * deathSourceW / referenceSourceW;
    outDrawH = def.drawHeight * deathSourceH / referenceSourceH;
    if (outDrawW <= 0) {
        outDrawW = def.drawWidth;
    }
    if (outDrawH <= 0) {
        outDrawH = def.drawHeight;
    }
}


bool TryDrawSnowApeKingDeath(
    const GameData::EnemyDefinition& def,
    int worldX,
    int worldY) {
    if (def.kind != GameData::EnemyKind::SnowApeKing) {
        return false;
    }

    int srcX = 0;
    int srcY = 0;
    int srcW = 0;
    int srcH = 0;
    const TCHAR* deathPath = AssetPaths::GetSnowApeKingEnemyDeathPath();
    if (!RenderUtils::GetImageFileVisibleBounds(deathPath, srcX, srcY, srcW, srcH)) {
        return false;
    }

    const int deathDrawH = def.drawHeight;
    const int deathDrawW = (srcW * deathDrawH) / srcH;
    if (deathDrawW <= 0 || deathDrawH <= 0) {
        return false;
    }

    const int deathSx = worldX - g_cameraX - deathDrawW / 2;
    const int deathSy = worldY - g_cameraY - deathDrawH / 2 + (def.drawHeight - deathDrawH) / 2;
    return RenderUtils::DrawImageFileRegion(
        deathPath,
        srcX,
        srcY,
        srcW,
        srcH,
        deathSx,
        deathSy,
        deathDrawW,
        deathDrawH);
}

CachedEnemyVisual& EnemyVisualCache(GameData::EnemyKind kind) {
    static std::array<CachedEnemyVisual, kEnemyKindCount> caches = {};
    CachedEnemyVisual& cache = caches[EnemyKindIndex(kind)];
    if (cache.loaded) {
        return cache;
    }

    const GameData::EnemyDefinition& def = GameData::GetEnemyDefinition(kind);
    RenderUtils::LoadAnimatedGif(cache.idleGif, def.idleGifPath);
    RenderUtils::LoadAnimatedGif(cache.walkGif, def.walkGifPath);
    RenderUtils::LoadAnimatedGif(cache.breakGif, AssetPaths::GetEliteBreakStatePath());
    RenderUtils::LoadImageFlexible(cache.deathImage, def.deathImagePath, 0, 0);
    cache.deathHasAlpha = RenderUtils::HasImage(cache.deathImage) && RenderUtils::HasMeaningfulAlpha(cache.deathImage);

    auto includeSize = [&](const RenderUtils::AnimatedGif& gif) {
        if (!gif.IsLoaded()) {
            return;
        }
        const int w = static_cast<int>(gif.image->GetWidth());
        const int h = static_cast<int>(gif.image->GetHeight());
        if (w > cache.referenceWidth) {
            cache.referenceWidth = w;
        }
        if (h > cache.referenceHeight) {
            cache.referenceHeight = h;
        }
    };
    includeSize(cache.idleGif);
    includeSize(cache.walkGif);

    cache.loaded = true;
    return cache;
}

bool DrawSnowApeKingDeathAt(int worldX, int worldY) {
    const GameData::EnemyDefinition& def =
        GameData::GetEnemyDefinition(GameData::EnemyKind::SnowApeKing);
    return TryDrawSnowApeKingDeath(def, worldX, worldY);
}

void DrawEnemyHeadLabel(
    int worldX,
    int worldY,
    int drawHeight,
    const GameData::EnemyDefinition& def,
    int hp,
    bool alive) {
    if (!alive) {
        return;
    }

    const int screenX = worldX - g_cameraX;
    const int headY = worldY - g_cameraY - drawHeight / 2;
    const int fontSize = (drawHeight >= 160) ? 24 : ((drawHeight >= 120) ? 20 : 16);

    TCHAR label[96] = { 0 };
    _stprintf_s(
        label,
        _countof(label),
        _T("%s HP：%d"),
        def.displayName,
        (hp > 0) ? hp : 0);

    setbkmode(TRANSPARENT);
    settextstyle(fontSize, 0, _T("黑体"));
    settextcolor(RGB(0, 0, 0));
    RECT labelRect = { screenX - 220, headY - 34, screenX + 220, headY - 6 };
    drawtext(label, &labelRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void DrawMoonMagicCageAtEnemyFeet(const EnemyInstance& runtime, const GameData::EnemyDefinition& def) {
    if (runtime.moonMagicCageEndTick <= RenderUtils::NowTickMs() ||
        !RenderUtils::HasImage(g_moonMagicCageImage)) {
        return;
    }

    const int drawW = def.drawWidth + kMoonMagicCageDrawPadding;
    const int drawH = max(24, (drawW * 2) / 3);
    const int footY = runtime.y + EnemyFootOffsetFromDefinition(def);
    const int drawX = runtime.x - g_cameraX - drawW / 2;
    const int drawY = footY - g_cameraY - drawH / 2;
    RenderUtils::DrawImageAuto(
        g_moonMagicCageImage,
        g_moonMagicCageHasAlpha,
        drawX,
        drawY,
        drawW,
        drawH);
}


void DrawIcefieldEnemyActor(const IcefieldEnemy& enemy) {
    const GameData::EnemyDefinition& def = GameData::GetEnemyDefinition(enemy.kind);
    CachedEnemyVisual& visual = EnemyVisualCache(enemy.kind);
    const EnemyInstance& runtime = enemy.runtime;

    if (!runtime.alive || runtime.hp <= 0) {
        int deathDrawW = def.drawWidth;
        int deathDrawH = def.drawHeight;
        ComputeEnemyDeathDrawSize(
            def,
            visual.deathImage,
            visual.referenceWidth,
            visual.referenceHeight,
            deathDrawW,
            deathDrawH);
        const int deathSx = runtime.x - g_cameraX - deathDrawW / 2;
        const int deathSy = runtime.y - g_cameraY - deathDrawH / 2 + (def.drawHeight - deathDrawH) / 2;
        RenderUtils::DrawImageAuto(visual.deathImage, visual.deathHasAlpha, deathSx, deathSy, deathDrawW, deathDrawH);
        return;
    }

    DrawMoonMagicCageAtEnemyFeet(runtime, def);

    RenderUtils::AnimatedGif& gif =
        (runtime.moving || runtime.attackState == EnemyInstance::AttackState::Charging) ? visual.walkGif : visual.idleGif;
    RenderUtils::UpdateAnimatedGifFrame(gif);

    int drawW = def.drawWidth;
    int drawH = def.drawHeight;
    if (gif.IsLoaded() && visual.referenceWidth > 0 && visual.referenceHeight > 0) {
        const int curW = static_cast<int>(gif.image->GetWidth());
        const int curH = static_cast<int>(gif.image->GetHeight());
        if (curW > 0 && curH > 0) {
            drawW = def.drawWidth * curW / visual.referenceWidth;
            drawH = def.drawHeight * curH / visual.referenceHeight;
            if (drawW <= 0) drawW = def.drawWidth;
            if (drawH <= 0) drawH = def.drawHeight;
        }
    }

    const int sx = runtime.x - g_cameraX - drawW / 2;
    const int sy = runtime.y - g_cameraY - drawH / 2 + (def.drawHeight - drawH) / 2;
    RenderUtils::DrawAnimatedGif(gif, sx, sy, drawW, drawH, !runtime.faceRight);
    DrawEnemyHeadLabel(runtime.x, runtime.y, def.drawHeight, def, runtime.hp, runtime.alive);

    if (runtime.breakStateActive && enemy.tier == GameData::EnemyTier::Elite) {
        RenderUtils::UpdateAnimatedGifFrame(visual.breakGif);
        const int breakDrawW = 56;
        const int breakDrawH = 56;
        const int breakX = runtime.x - g_cameraX - breakDrawW / 2;
        const int breakY = sy - breakDrawH / 3;
        RenderUtils::DrawAnimatedGif(visual.breakGif, breakX, breakY, breakDrawW, breakDrawH, false);
    }
}

}  

void DrawIcefieldActorsSorted() {
    if (g_icefieldEnemies.empty() && IsIcefieldNormalBattleActive()) {
        DrawMapObstaclesLayered(INT_MIN, INT_MAX);
        DrawEnergyDropsLayered(INT_MIN, INT_MAX);
        DrawEnemy();
        DrawPlayer();
        return;
    }

    struct ActorEntry {
        int footY = 0;
        bool isPlayer = false;
        bool isBoss = false;
        size_t enemyIndex = 0;
    };

    std::vector<bool> allowOnScreen;
    const bool bossVisible =
        (g_battlePhase == BattlePhase::BossFight) && IsBossVisibleOnScreenByState(g_enemy);
    BuildIcefieldVisibleMask(bossVisible, allowOnScreen);

    std::vector<ActorEntry> actors;
    actors.reserve(g_icefieldEnemies.size() + 2U);
    actors.push_back({ g_playerY + kPlayerFootOffsetY, true, false, 0U });
    if (!IsIcefieldNormalBattleActive()) {
        const int bossFootOffsetY = EnemyFootOffsetFromDefinition(EnemyDef());
        const int bossFootY = g_enemy.y + bossFootOffsetY;
        actors.push_back({ bossFootY, false, true, 0U });
    }

    for (size_t i = 0; i < g_icefieldEnemies.size(); ++i) {
        const IcefieldEnemy& enemy = g_icefieldEnemies[i];
        if (!enemy.runtime.alive || enemy.runtime.hp <= 0) {
            continue;
        }
        if (i >= allowOnScreen.size() || !allowOnScreen[i]) {
            continue;
        }
        ActorEntry entry;
        const GameData::EnemyDefinition& enemyDef = GameData::GetEnemyDefinition(enemy.kind);
        entry.footY = enemy.runtime.y + EnemyFootOffsetFromDefinition(enemyDef);
        entry.isPlayer = false;
        entry.isBoss = false;
        entry.enemyIndex = i;
        actors.push_back(entry);
    }

    std::sort(
        actors.begin(),
        actors.end(),
        [](const ActorEntry& a, const ActorEntry& b) { return a.footY < b.footY; });

    const GameData::EnemyKind previousKind = g_enemyKind;
    const EnemyInstance previousEnemy = g_enemy;
    const GameData::EnemyCombatParams previousEnemyParams = g_enemyParams;
    const bool previousEnemySawPlayerLastFrame = g_enemySawPlayerLastFrame;
    const int previousActiveIndex = g_activeIcefieldEnemyIndex;

    int layerMinY = INT_MIN;
    bool deferBossDeathDraw = false;
    for (const ActorEntry& actor : actors) {
        DrawMapObstaclesLayered(layerMinY, actor.footY);
        DrawEnergyDropsLayered(layerMinY, actor.footY);

        if (actor.isPlayer) {
            DrawPlayer();
            layerMinY = actor.footY;
            continue;
        }
        if (actor.isBoss) {
            if (!g_enemy.alive || g_enemy.hp <= 0) {
                deferBossDeathDraw = true;
                layerMinY = actor.footY;
                continue;
            }
            DrawEnemy();
            layerMinY = actor.footY;
            continue;
        }

        const IcefieldEnemy& enemy = g_icefieldEnemies[actor.enemyIndex];
        DrawIcefieldEnemyActor(enemy);
        layerMinY = actor.footY;
    }
    DrawMapObstaclesLayered(layerMinY, INT_MAX);
    DrawEnergyDropsLayered(layerMinY, INT_MAX);
    if (deferBossDeathDraw) {
        DrawSnowApeKingDeathAt(g_enemy.x, g_enemy.y);
    }

    g_enemyKind = previousKind;
    g_enemy = previousEnemy;
    g_enemyParams = previousEnemyParams;
    g_enemySawPlayerLastFrame = previousEnemySawPlayerLastFrame;
    g_activeIcefieldEnemyIndex = previousActiveIndex;
}

void DrawEnemy() {
    const GameData::EnemyDefinition& enemyDef = EnemyDef();

    if (!g_enemy.alive) {
        if (enemyDef.kind == GameData::EnemyKind::SnowApeKing &&
            DrawSnowApeKingDeathAt(g_enemy.x, g_enemy.y)) {
            return;
        }

        int deathDrawW = enemyDef.drawWidth;
        int deathDrawH = enemyDef.drawHeight;
        ComputeEnemyDeathDrawSize(
            enemyDef,
            g_enemyDeathImage,
            g_enemyAnimReferenceWidth,
            g_enemyAnimReferenceHeight,
            deathDrawW,
            deathDrawH);
        const int deathSx = g_enemy.x - g_cameraX - deathDrawW / 2;
        const int deathSy = g_enemy.y - g_cameraY - deathDrawH / 2 + (enemyDef.drawHeight - deathDrawH) / 2;
        RenderUtils::DrawImageAuto(g_enemyDeathImage, g_enemyDeathHasAlpha, deathSx, deathSy, deathDrawW, deathDrawH);
        return;
    }

    AnimatedGif* gifPtr = nullptr;
    if (g_enemy.bossCrossJumping && g_enemyJumpGif.IsLoaded()) {
        gifPtr = &g_enemyJumpGif;
    }
    else if (g_enemy.bossRoaring && g_enemySpecialAttackGif.IsLoaded()) {
        gifPtr = &g_enemySpecialAttackGif;
    }
    else {
        const bool useWalkAnimation =
            g_enemy.moving || g_enemy.attackState == EnemyInstance::AttackState::Charging;
        gifPtr = useWalkAnimation ? &g_enemyWalkGif : &g_enemyIdleGif;
    }

    AnimatedGif& gif = *gifPtr;
    const int referenceSourceW = g_enemyAnimReferenceWidth;
    const int referenceSourceH = g_enemyAnimReferenceHeight;
    const int currentSourceW = gif.IsLoaded() ? static_cast<int>(gif.image->GetWidth()) : 0;
    const int currentSourceH = gif.IsLoaded() ? static_cast<int>(gif.image->GetHeight()) : 0;

    int drawW = enemyDef.drawWidth;
    int drawH = enemyDef.drawHeight;
    if (currentSourceW > 0 && currentSourceH > 0 && referenceSourceW > 0 && referenceSourceH > 0) {
        drawW = enemyDef.drawWidth * currentSourceW / referenceSourceW;
        drawH = enemyDef.drawHeight * currentSourceH / referenceSourceH;
        if (drawW <= 0) {
            drawW = enemyDef.drawWidth;
        }
        if (drawH <= 0) {
            drawH = enemyDef.drawHeight;
        }
    }

    const int sx = g_enemy.x - g_cameraX - drawW / 2;
    const int sy = g_enemy.y - g_cameraY - drawH / 2 + (enemyDef.drawHeight - drawH) / 2;
    DrawMoonMagicCageAtEnemyFeet(g_enemy, enemyDef);
    RenderUtils::UpdateAnimatedGifFrame(gif);
    RenderUtils::DrawAnimatedGif(gif, sx, sy, drawW, drawH, !g_enemy.faceRight);
    DrawEnemyHeadLabel(g_enemy.x, g_enemy.y, enemyDef.drawHeight, enemyDef, g_enemy.hp, g_enemy.alive);

    if (g_enemy.breakStateActive && enemyDef.tier == GameData::EnemyTier::Elite) {
        RenderUtils::UpdateAnimatedGifFrame(g_enemyBreakGif);
        const int breakDrawW = 56;
        const int breakDrawH = 56;
        const int breakX = g_enemy.x - g_cameraX - breakDrawW / 2;
        const int breakY = sy - breakDrawH / 3;
        RenderUtils::DrawAnimatedGif(g_enemyBreakGif, breakX, breakY, breakDrawW, breakDrawH, false);
    }
}


}  
