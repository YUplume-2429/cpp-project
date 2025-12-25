#ifndef TANK_H
#define TANK_H

#include <QPixmap>
#include <QVector>
#include <QPoint>
#include <QPainter>

class Bullet;

class Tank {
public:
    enum class Direction { Up, Down, Left, Right };

    explicit Tank(int level = 1);

    void setPosition(const QPoint& p) { m_x = p.x(); m_y = p.y(); }
    QPoint position() const { return QPoint(m_x, m_y); }
    int x() const { return m_x; }
    int y() const { return m_y; }

    void setDirection(Direction d) { m_direction = d; }
    Direction direction() const { return m_direction; }

    int level() const { return m_level; }
    int health() const { return m_health; }
    int maxHealth() const { return m_maxHealth; }
    int speed() const { return m_speed; }

    bool blasted() const { return m_blasted; }
    bool blastOver() const { return m_blastOver; }

    int lastFireTimeMs() const { return m_lastFireTimeMs; }
    void setLastFireTimeMs(int t) { m_lastFireTimeMs = t; }
    int fireIntervalMs() const { return m_fireIntervalMs; }

    void move();
    void takeDamage(int amount);
    void drawBlast(QPainter& painter);
    void die();
    Bullet* shoot();

private:
    void loadBlastImages();
    void playBlastSound(); // 占位：可接入 Qt Multimedia
    void playFireSound();  // 占位：可接入 Qt Multimedia

private:
    int m_x{0};
    int m_y{0};
    Direction m_direction{Direction::Up};

    int m_level{1};
    int m_maxHealth{1};
    int m_health{1};
    int m_speed{2};
    int m_bulletSpeed{1};

    bool m_blasted{false};
    bool m_blastOver{false};
    int m_blastFrame{0};
    int m_blastMaxFrame{0};

    int m_lastFireTimeMs{0};
    int m_fireIntervalMs{50};

    QVector<QPixmap> m_boomImages;
};

#endif // TANK_H
