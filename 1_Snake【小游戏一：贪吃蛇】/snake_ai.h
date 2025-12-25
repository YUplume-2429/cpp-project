#ifndef SNAKE_AI_H
#define SNAKE_AI_H

#include <QQueue>
#include <QRect>
#include <QList>
#include <QPoint>
#include <QSet>
#include <QTimer>
#include <QHash>
#include <algorithm>
#include <queue>
#include <map>


inline bool operator<(const QPoint& a, const QPoint& b) {
    if (a.x() != b.x()) return a.x() < b.x();
    return a.y() < b.y();
}


inline uint qHash(const QPoint& point) {
    return qHash(point.x()) ^ (qHash(point.y()) << 1);
}

class snake; // 前向声明

struct AStarNode {
    QPoint position;
    int gCost; // 从起点到当前节点的代价
    int hCost; // 从当前节点到终点的启发式代价
    int fCost() const { return gCost + hCost; }
    AStarNode* parent;
    
    AStarNode(QPoint pos, int g = 0, int h = 0, AStarNode* p = nullptr)
        : position(pos), gCost(g), hCost(h), parent(p) {}
    
    bool operator<(const AStarNode& other) const {
        return fCost() > other.fCost(); // 优先队列中小的在前
    }
};

struct PathTarget {
    QPoint position;
    int priority; // 优先级：特殊食物(100) > 普通食物(50) > 加速道具(30) > 减速道具(20)
    QString type;
    
    PathTarget(QPoint pos, int pri, QString t) 
        : position(pos), priority(pri), type(t) {}
    
    bool operator<(const PathTarget& other) const {
        return priority < other.priority; // 优先级队列中大的在前
    }
};

class SnakeAI
{
public:
    SnakeAI(snake* game);
    ~SnakeAI();
    
    // 主要AI函数
    int getNextDirection(); // 获取下一步移动方向
    void updateGameState(); // 更新游戏状态
    
    // 寻路算法
    QList<QPoint> findPathAStar(QPoint start, QPoint goal);
    QList<QPoint> findPathBFS(QPoint start, QPoint goal);
    
    // 路径评估和安全检查
    bool isPositionSafe(QPoint pos);
    bool willCreateDeadEnd(QPoint pos, int direction);
    int calculateSafetyScore(QPoint pos);
    
    // 目标选择
    PathTarget selectBestTarget();
    QList<PathTarget> getAllTargets();
    
    // 工具函数
    QPoint rectToGridPoint(const QRect& rect);
    QRect gridPointToRect(const QPoint& point);
    int manhattanDistance(QPoint a, QPoint b);
    bool isValidPosition(QPoint pos);
    bool isOccupied(QPoint pos);
    
    // 生存策略
    int getEmergencyDirection(); // 紧急情况下的方向选择
    bool canReachTail(); // 检查是否能到达尾部（避免死循环）
    QPoint findSafestNearbyPosition(QPoint start); // 寻找附近最安全的位置
    
    // 安全性评估辅助函数
    int countEscapeRoutes(QPoint pos); // 计算指定位置的出路数量
    bool isBoomPosition(QPoint pos); // 检查位置是否有炸弹
    
private:
    snake* gameInstance;
    QTimer* aiTimer;
    
    // AI参数
    static const int GRID_SIZE = 20;
    static const int GAME_AREA_X = 480;
    static const int GAME_AREA_Y = 180;
    static const int GAME_AREA_WIDTH = 640;
    static const int GAME_AREA_HEIGHT = 640;
    static const int GRID_WIDTH = 32;  // 640/20
    static const int GRID_HEIGHT = 32; // 640/20
    
    // AI状态
    QList<QPoint> currentPath;
    QPoint lastTarget;
    int stuckCounter; // 被困计数器
    
    // 方向常量 1:UP, 2:DOWN, 3:LEFT, 4:RIGHT
    QList<QPoint> directions = {
        QPoint(0, -1), // UP
        QPoint(0, 1),  // DOWN
        QPoint(-1, 0), // LEFT
        QPoint(1, 0)   // RIGHT
    };
};

#endif // SNAKE_AI_H
