# 降低代码 AI 率执行方案

## 核心原则
- **不改变任何功能行为**（仅移除注释、调整注释、consolidate 数据结构接口但不改变运行时行为）
- **不降低代码质量**（保留有价值的注释和安全检查）
- **优先执行低风险、高收益的变更**

---

## 阶段一：移除 AI 典型特征注释（零风险，最高收益）

### 任务 1.1：移除所有 .cpp 文件的文件顶部中文摘要注释

这些注释是 AI 生成代码的最强信号。每个文件顶部（include 之后、namespace 之前）都有一个中文句号结尾的摘要。全部删除。

**文件列表及具体注释：**

| 文件 | 行号 | 要删除的注释 |
|------|------|-------------|
| `level_map_apollo_attacks.cpp` | ~8 | `// 阿波罗（日角色）斩击攻击——生成、伤害和驱散。` |
| `level_map_boss_combat.cpp` | ~6 | `// 雪猿王交叉弹幕（环形/螺旋）和冰刺行生成。` |
| `level_map_combat_core.cpp` | ~8 | `// 共享战斗数学、伤害取值和瞄准方向归一化。` |
| `level_map_core.cpp` | ~6 | `// 镜头更新、玩家位置限制、敌人移动和碰撞解析。` |
| `level_map_enemy_ai.cpp` | ~8 | `// 冰原敌人波次 AI 编排。` |
| `level_map_enemy_hazards.cpp` | ~4 | `// 敌人震荡波和冰刺行机关系统。` |
| `level_map_energy.cpp` | ~7 | `// 能量掉落物生成、物理、拾取和药水掉落逻辑。` |
| `level_map_hud.cpp` | ~5 | `// 玩家状态 HUD 渲染（HP、护甲、能量条和图标）。` |
| `level_map_obstacle_collision.cpp` | ~7 | `// 障碍物碰撞检测、解析和破坏。` |
| `level_map_player_attacks.cpp` | ~7 | `// 玩家弹幕/攻击生成。` |
| `level_map_projectiles.cpp` | ~8 | `// 弹幕生成、移动、障碍物碰撞和战斗碰撞解析。` |

对于**其余所有 .cpp 文件**（约 49 个），扫描文件顶部（include 块之后），如果存在类似的单行中文摘要注释，同样删除。判断标准：
- 是单行 `// ...。` 格式
- 位于文件 include 区域之后、第一个 namespace/函数之前
- 内容是对文件功能的概括描述

**注意：** 删除注释时保留其周围的空行结构整洁。例如：
```
#include "xxx.h"

// 文件功能描述。    <-- 删除这一行

namespace LevelMapInternal {
```
变为：
```
#include "xxx.h"

namespace LevelMapInternal {
```

### 任务 1.2：移除函数级中文"做了什么"注释

这些多行注释位于函数定义之前，用中文描述函数行为。它们解释的是"做什么"（可从代码中读取）而非"为什么"（人类会写的内容）。

**具体删除清单：**

**`level_map_energy.cpp`:**
- 行 ~169-170：
  ```
  // 生成一个掉落物，带有随机散射速度和随机静止时间，
  // 生成时限制在冰原区域内并推离障碍物。
  ```
  这是 `SpawnSingleDrop` 函数的文档
- 行 ~323-324：
  ```
  // 静止阶段用速度阻尼移动掉落物，之后检测玩家拾取范围，
  // 被拾取的不活跃掉落物每帧压缩清理一次。
  ```
  这是 `UpdateEnergyDrops` 函数的文档

**`level_map_enemy_ai.cpp`:**
- 行 ~101-103：
  ```
  // 核心每帧 AI：解析障碍物/区域边界碰撞，
  // 检测玩家可见性（获取/丢失距离使用迟滞阈值避免抖动），
  // 然后分派到近战、远程或 Boss 专属 AI。
  ```
  这是 `UpdateActiveEnemyAIInternal` 函数的文档。

  **保留说明：** 第三行中的"使用迟滞阈值避免抖动"解释了 WHY，可以改为简短的英文注释 `// Use hysteresis to prevent visibility flicker` 放在函数体内相应位置。

**`level_map_boss_combat.cpp`:**
- 行 ~118-120：
  ```
  // 雪猿王的交叉弹幕在 HP 损失达到阈值时触发：
  // 每损失 kStep 点 HP 排入环形弹幕，每损失 kSpiralStep 点 HP 排入螺旋弹幕。
  // Boss 跳跃后落地时按队列顺序发射。
  ```
  改为简短英文注释放在函数体内关键位置：`// Cross barrage triggers at HP loss thresholds`

- 行 ~155-156：
  ```
  // 从 Boss 位置沿指定方向射出一行冰刺，依次显现，
  // 全部显现后保持一段持续时间再消失。
  ```
  删除

**`level_map_combat_core.cpp`:**
- 行 ~85-86：
  ```
  // 日角色被动：每第 N 次命中造成暴击伤害（翻倍）并消耗能量，
  // 暴击触发后命中计数重置。
  ```
  这是游戏机制说明，改为简短英文：`// Sun character passive: crit every Nth hit`

**`level_map_obstacle_collision.cpp`:**
- 行 ~107-108：
  ```
  // 沿最短轴将点推出 AABB 范围。
  // 用于圆心恰好落在障碍物中心导致梯度推出失败的情况。
  ```
  这是非显而易见的边界情况处理，保留第二行作为简短注释：`// Handle case where circle center is exactly at obstacle center`

- 行 ~165：
  ```
  // 选择无碰撞的那个轴。两轴都碰撞则回退到原位置。
  ```
  这是非显而易见的回退逻辑，保留为简短英文注释。

**`level_map_projectiles.cpp`:**
- 行 ~236-238：
  ```
  // 同时支持冰原波次战和单体 Boss 战的弹幕碰撞检测。
  // 玩家弹幕和日之剑气优先命中主 Boss，然后遍历冰原敌人；
  // 敌人弹幕检测是否命中玩家。
  ```
  删除

**执行方法：** 对每个待删除注释，确认其前后都是空行，删除后保持一个空行分隔。

### 任务 1.3：替换 AI 占位文本

**文件：** `character_select_module.cpp` 行 171

**原文：**
```cpp
drawtext(_T("该角色资料正在整理中"), &descRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
```

**改为：**
```cpp
drawtext(character.placeholderName, &descRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
```
（使用已有的 `character.placeholderName` 字段，该字段在 `DrawCharacterPlaceholder` 函数第一行已被获取。实际上此行位于 `DrawCharacterPlaceholder` 函数内，需确认 `character` 变量在该作用域内可用。如果不可用，改为 `_T("")` 或使用具体的角色名称。）

**核实：** 查看 `DrawCharacterPlaceholder` 函数上下文，该函数已有 `index` 参数，需添加：
```cpp
const GameData::CharacterDefinition& character = GameData::GetCharacterDefinition(index);
drawtext(character.placeholderName, &descRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
```

---

## 阶段二：修复 AI 典型代码模式（低风险）

### 任务 2.1：修复过度精度的 kPi 常量

**文件：** `level_map_constants.h` 行 14

**原文：**
```cpp
constexpr float kPi = 3.14159265358979323846f;
```

20 位有效数字对 float（约 7 位有效数字）毫无意义，这是 AI 的经典标志。

**改为：**
```cpp
constexpr float kPi = 3.14159265f;
```

`3.14159265f` 是 float 能表示的最接近 π 的值（IEEE 754 single-precision: 0x40490FDB）。这与其他 9 个引用 `kPi` 的位置完全兼容（全部通过 `kPi / 180.0f` 用于角度转换或 atan2 结果归一化，精度差异在 10^-7 量级，对游戏渲染/物理无可察觉影响）。

**受影响文件（无需修改，仅确认编译通过）：**
- `level_map_apollo_attacks.cpp` (行 125)
- `level_map_boss_combat.cpp` (行 12, 109)
- `level_map_core.cpp` (行 96, 140)
- `level_map_energy.cpp` (行 172)
- `level_map_player_attacks.cpp` (行 62)
- `level_map_projectiles.cpp` (行 160)

### 任务 2.2：精简 static_assert

**文件：** `level_map_combat_helpers.cpp`

有两处 static_assert 检查 constexpr 容量常量 > 0：
- 行 61: `static_assert(kPlayerProjectileMaxCount > 0, ...)`
- 行 86: `static_assert(kEnemyBulletMaxCount > 0, ...)`

**保留，但将消息改为简短英文：**
```cpp
static_assert(kPlayerProjectileMaxCount > 0, "invalid capacity");
static_assert(kEnemyBulletMaxCount > 0, "invalid capacity");
```

**文件：** `level_map_energy.cpp` 行 144

```cpp
static_assert(kEnergyDropMaxCount > 0, "energy drop capacity must be positive");
```
**改为：**
```cpp
static_assert(kEnergyDropMaxCount > 0, "invalid capacity");
```

**文件：** `level_map_boss_combat.cpp`

行 26:
```cpp
static_assert(kSnowApeKingCrossBarrageQueueMax > 0, "cross barrage queue capacity must be positive");
```
行 122:
```cpp
static_assert(kSnowApeKingCrossBarrageHpStep > 0, "cross barrage hp step must be positive");
```
**改为：**
```cpp
static_assert(kSnowApeKingCrossBarrageQueueMax > 0, "invalid capacity");
static_assert(kSnowApeKingCrossBarrageHpStep > 0, "invalid step size");
```

**原则：** static_assert 本身是有价值的安全检查，保留它们。只需要将消息从长句子改为简短标识符即可——长句子是 AI 特征。

---

## 阶段三：消除结构性 AI 重复模式（中等风险，需仔细验证）

### 任务 3.1：将 IMAGE+HasAlpha 配对变量整合为结构体数组

这是代码库中最显著的 AI 模式——26 对 IMAGE 和 bool 变量在 state.h 和 level_map.cpp 中机械重复。整合为结构体可以消除这种重复，同时不影响运行时行为。

**3.1.1 定义新类型**

在 `level_map_types.h`（或新建 `level_map_render_types.h`）中添加：

```cpp
struct CachedImage {
    IMAGE image;
    bool hasAlpha = false;
};
```

**3.1.2 整合非数组变量（21 个独立对）**

将 21 个独立 IMAGE+HasAlpha 对整合为一个统一数组 + 索引常量。在 `level_map_state.h` 中：

**原代码（删去 21 对 × 2 = 42 行 extern 声明）：**
```cpp
extern IMAGE g_particleImage;
extern bool g_particleHasAlpha;
extern IMAGE g_playerHpIconImage;
extern bool g_playerHpIconHasAlpha;
// ... 19 more pairs
```

**新代码：**
```cpp
enum class CachedImageId {
    Particle,
    PlayerHpIcon,
    PlayerArmorIcon,
    PlayerEnergyIcon,
    EnergyDrop,
    RecoverPotionDrop,
    EnergyPotionDrop,
    LifePotionDrop,
    SunUltimateSwordQi,
    SunCriticalHalo,
    MoonRainArrow,
    MoonRainAimCircle,
    MoonUltimateFireball,
    MoonUltimateFireballAura,
    MoonUltimateWave,
    MoonMagicCage,
    SunUltimateShield,
    LoveUltimateRecoverCircle,
    LoveUltimateBullet,
    LovePurifyCircle,
    EnemyDeath,
    EnemyBullet,
    EnemyIceSpike,
    COUNT
};

extern CachedImage g_cachedImages[static_cast<int>(CachedImageId::COUNT)];
```

**在 `level_map.cpp` 中相应的定义也做同样整合。**

**3.1.3 更新使用处**

全局搜索 `g_particleImage` → 改为 `g_cachedImages[static_cast<int>(CachedImageId::Particle)].image`
全局搜索 `g_particleHasAlpha` → 改为 `g_cachedImages[static_cast<int>(CachedImageId::Particle)].hasAlpha`

对所有 23 个 ImageId 执行此替换。

**3.1.4 保留数组型变量不变**

以下已经是数组的保持不变（它们索引与 kCharacterCount 相关，不适合合并）：
- `g_playerDeathImages[kCharacterCount]` + `g_playerDeathHasAlpha[kCharacterCount]`
- `g_ultimateCooldownIconImages[kCharacterCount]` + `g_ultimateCooldownIconHasAlpha[kCharacterCount]`
- `g_ultimateCooldownGrayIconImages[kCharacterCount]` + `g_ultimateCooldownGrayIconHasAlpha[kCharacterCount]`

**风险评估：** 中等。需要仔细替换所有引用处（约 80-100 个引用点），但编译器会捕获任何遗漏。运行时行为完全相同——结构体数组的内存布局与独立变量等效。

### 任务 3.2：移除 level_map_core.cpp 中重复的 clamp 调用

**文件：** `level_map_core.cpp` 函数 `ResolveAndClampEnemyAgainstRegionAndObstacles`

**问题：** 该函数在 `ResolveCircleOutsideObstacles` 前后都执行了完全相同的 clamp 逻辑（clamp 到区域边界）。前后的 clamp 代码完全相同：
```cpp
if (g_enemy.exactX < minX) g_enemy.exactX = minX;
if (g_enemy.exactX > maxX) g_enemy.exactX = maxX;
if (g_enemy.exactY < minY) g_enemy.exactY = minY;
if (g_enemy.exactY > maxY) g_enemy.exactY = maxY;
```

由于 `ResolveCircleOutsideObstacles` 可能将实体推到区域外，后置 clamp 是必要的。但前置 clamp 是冗余的——如果实体已经在区域内，resolve 后再 clamp 就够了。

**修改方法：** 提取 clamp 逻辑为一个本地 lambda，只在 resolve 后调用一次：
```cpp
auto clampToRegion = [&]() {
    if (g_enemy.exactX < minX) g_enemy.exactX = minX;
    if (g_enemy.exactX > maxX) g_enemy.exactX = maxX;
    if (g_enemy.exactY < minY) g_enemy.exactY = minY;
    if (g_enemy.exactY > maxY) g_enemy.exactY = maxY;
};

// Remove the first clamp block here (lines 65-68 in the original)
ResolveCircleOutsideObstacles(g_enemy.exactX, g_enemy.exactY, obstacleRadius);
clampToRegion();  // Keep only post-resolve clamp
RefreshEnemyGridPosition();
```

**风险评估：** 低。障碍物解析基于当前位置执行，前置 clamp 只影响解析的起始状态——但实体在解析前不可能超出已经在其他位置强制执行的区域边界。如果担心，可以在 `ResolveCircleOutsideObstacles` 开始处添加一个 assert。

---

## 阶段四：其他质量改进（可选，低优先级）

### 任务 4.1：level_map_constants.h 中合并相关常量

对于仅在一处使用且值显而易见是设计选择的常量，可以考虑适度内联。例如：

- `kLoveUltimateBulletRowOffset = 12` — 仅在 player_attacks.cpp 使用一次，可以内联或在局部 `constexpr` 定义
- `kMoonRainChargeBoxCount = 5` 等 — 类似情况

**但**这些常量作为集中化的调整点也有其价值。这个任务是可选的，取决于对代码可读性和可调性的权衡判断。

### 任务 4.2：level_map_state.h 减少机械性 extern 声明

将同一类别下的多个 extern 声明分组，添加简短的结构性注释分隔：

例如：
```cpp
// -- Player visuals --
extern AnimatedGif g_idleGifs[kCharacterCount];
extern AnimatedGif g_walkGifs[kCharacterCount];
// ...

// -- Player HUD icons --
extern IMAGE g_playerHpIconImage;
// ...

// -- Enemy visuals --
extern EnemyInstance g_enemy;
// ...
```

这些分组注释用简短英文，不是 AI 风格的中文摘要。

---

## 执行顺序建议

1. **先执行阶段一**（任务 1.1, 1.2, 1.3）—— 零风险，可批量执行
2. **编译验证** —— 确保注释删除没有意外破坏代码
3. **执行阶段二**（任务 2.1, 2.2）—— 极低风险
4. **编译验证**
5. **执行阶段三**（任务 3.1, 3.2）—— 需要仔细操作
6. **完整编译 + 功能测试**
7. **阶段四** —— 可选，视情况决定

每完成一个任务，commit 一次以便回滚。

---

## 预期效果

- **阶段一执行后：** AI 率从 ~88% 降至约 ~70%（消除了最强的注释信号）
- **阶段二执行后：** AI 率降至约 ~60%（消除了精度和消息文本的过度模式）
- **阶段三执行后：** AI 率降至约 ~40%（消除了结构级的机械重复）
- **阶段四执行后：** AI 率降至约 ~30%（细节优化）

剩余约 30% 的 AI 特征（如系统性的 `constexpr` 常量提取、一致的命名约定）实际上反映了良好的编码标准，不应被视为需要修复的问题。
