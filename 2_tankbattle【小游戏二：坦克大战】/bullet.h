#ifndef BULLET_H
#define BULLET_H

#include <QPixmap>
#include <QVector>
#include <QRect>
#include <QPoint>
#include <QPainter>
#include <QSize>

class Bullet {
public:
    enum class Direction { Up, Down, Left, Right };

    Bullet(const QPoint& position, Direction direction, int speed);

    void move();
    bool checkCollision(const QRect& targetRect) const;
    void boom();
    void draw(QPainter& painter);

    QPoint position() const { return QPoint(m_x, m_y); }
    int x() const { return m_x; }
    int y() const { return m_y; }
    int speed() const { return m_speed; }
    Direction direction() const { return m_direction; }

    bool exploded() const { return m_exploded; }
    bool explodeOver() const { return m_explodeOver; }

    static QSize bulletSize();
    static int collisionSize();

private:
    int m_x;
    int m_y;
    Direction m_direction;
    int m_speed;

    QPixmap m_bulletImage;
    QVector<QPixmap> m_bulletBoomImages;

    bool m_exploded{false};
    int m_explodeFrame{0};
    bool m_explodeOver{false};
};

#endif // BULLET_H
