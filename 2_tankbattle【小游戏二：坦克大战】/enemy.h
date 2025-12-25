#ifndef ENEMY_H
#define ENEMY_H

#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QPainter>
#include <QVector>
#include <QString>
#include "tank.h"

class GameMap; // 前向声明，需提供 getTile/checkCollision 接口

class EnemyTank : public Tank {
public:
    enum class Behavior { Patrol, Aggressive };
    using Direction = Tank::Direction;

    // 依赖：level 为 1/2（对应两套贴图），tileSize 为地图格子像素尺寸
    EnemyTank(const QPoint& position, int level, int tileSize);

    void move(const QPoint* playerPosition = nullptr, const GameMap* gameMap = nullptr);
    Bullet* shoot(); // 返回生成的子弹指针（可能为 nullptr）
    void takeDamage(int amount);
    void die();
    void update();
    void draw(QPainter& painter);
    void onHitFeedback();

private:
    bool detectPlayer(const QPoint& playerPosition, const GameMap* gameMap) const;
    void patrol(const GameMap* gameMap);
    void chasePlayer(const QPoint& playerPosition, const GameMap* gameMap);
    void tryMove(const GameMap* gameMap);

    // 资源与绘制
    const QPixmap& spriteFor(Direction d) const;

private:
    // 使用基类 Tank 的方向，不再维护重复成员
    bool m_moving{false};
    int m_moveStartTime{0};
    int m_moveDurationMs{500};
    QPoint m_moveStartPos{0, 0};
    QPoint m_moveTarget{0, 0};

    // 行为
    Behavior m_behavior{Behavior::Patrol};

    // 爆炸状态改由 Tank 基类管理（blasted/blastOver/drawBlast）
    // 受击反馈（闪烁计时，毫秒或tick）
    int m_hitFlashTicks{0};

    // 射击与存弹
    int m_storedBullet{0};
    int m_maxStoreBullets{0};
    // 开火节流（独立于 Tank 的节流，避免影响玩家）
    int m_enemyLastShootMs{0};
    int m_enemyFireIntervalMs{400}; // 敌人每 400ms 最多一发

    // 贴图资源：按 level 选择集合
    QPixmap m_enemyD;
    QPixmap m_enemyL;
    QPixmap m_enemyR;
    QPixmap m_enemyU;

    // 地图格子尺寸
    int m_tileSize{32};

    // 视野探测曼哈顿距离
    static constexpr int kDetectRange = 7;
};

#endif // ENEMY_H
