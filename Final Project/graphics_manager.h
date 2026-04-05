#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

class GraphicsManager {
public:
    static void initialize();//创建游戏窗口
    static void cleanup();//关闭图形窗口释放资源
    static void clearScreen();//清空屏幕内容
};

#endif
