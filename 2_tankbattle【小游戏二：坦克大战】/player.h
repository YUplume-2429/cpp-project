#ifndef PLAYER_H
#define PLAYER_H

#include <QPixmap>
#include <QPoint>
#include <QPainter>
#include <QString>
#include "tank.h"

class GameMap; // 需要 checkCollision
class Bullet;  // 已实现于 bullet.h

class PlayerTank : public Tank {
public:
    using Direction = Tank::Direction;

    PlayerTank(const QPoint& position, int level = 1, int lives = 2, int score = 0, Direction direction = Direction::Up);

    void move(Direction direction, const GameMap* gameMap);
    Bullet* shoot();

    void increaseScore(int points);
    int getScore() const { return m_score; }
    int getLevel() const { return m_level; }
    int getHealth() const { return m_health; }

    bool takeDamage(int dmg);
    void dieOnce(const QPoint& position);
    void reset(const QPoint& position);

    void draw(QPainter& painter);

    bool alive() const { return m_alive; }

private:
    void reloadImages();

private:
    Direction m_direction{Direction::Up};

    int m_level{1};
    int m_lives{2};
    int m_score{0};
    int m_killCount{0};
    bool m_alive{true};

    int m_health{0};

    // 爆炸状态（沿用 Tank 的爆炸帧绘制方法）
    bool m_blasted{false};
    int m_explodeFrame{0};
    int m_blastMaxFrame{0};
    bool m_blastOver{false};

    // 资源
    QPixmap m_heroD;
    QPixmap m_heroL;
    QPixmap m_heroR;
    QPixmap m_heroU;
};

#endif // PLAYER_H
