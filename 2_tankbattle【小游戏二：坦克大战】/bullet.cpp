#include "bullet.h"
#include <QString>

namespace {
const char* kBulletImagePath = ":/bullet/images/bullet/bullet.png";
const char* kBoomPaths[] = {
    ":/bullet_boom/images/bullet_boom/bomb_1.gif",
    ":/bullet_boom/images/bullet_boom/bomb_2.gif",
    ":/bullet_boom/images/bullet_boom/bomb_3.gif",
};

const QSize kBulletDrawSize(4, 4);
const QSize kBoomDrawSize(32, 32);
const int kBulletCollisionSize = 16;
}

Bullet::Bullet(const QPoint& position, Direction direction, int speed)
    : m_x(position.x()), m_y(position.y()), m_direction(direction), m_speed(speed) {
    QPixmap bulletPixmap(QString::fromUtf8(kBulletImagePath));
    if (!bulletPixmap.isNull()) {
        m_bulletImage = bulletPixmap.scaled(kBulletDrawSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    } else {
        m_bulletImage = QPixmap(kBulletDrawSize);
        m_bulletImage.fill(Qt::white);
    }

    for (const char* path : kBoomPaths) {
        QPixmap boomPixmap(QString::fromUtf8(path));
        if (!boomPixmap.isNull()) {
            m_bulletBoomImages.append(boomPixmap.scaled(kBoomDrawSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        } else {
            QPixmap fallback(kBoomDrawSize);
            fallback.fill(Qt::transparent);
            m_bulletBoomImages.append(fallback);
        }
    }
}

void Bullet::move() {
    if (m_exploded) return;

    switch (m_direction) {
    case Direction::Up:
        m_y -= m_speed;
        break;
    case Direction::Down:
        m_y += m_speed;
        break;
    case Direction::Left:
        m_x -= m_speed;
        break;
    case Direction::Right:
        m_x += m_speed;
        break;
    }
}

bool Bullet::checkCollision(const QRect& targetRect) const {
    if (m_exploded) return false;
    QRect bulletRect(m_x, m_y, kBulletCollisionSize, kBulletCollisionSize);
    return bulletRect.intersects(targetRect);
}

void Bullet::boom() {
    m_exploded = true;
}

void Bullet::draw(QPainter& painter) {
    if (!m_exploded) {
        painter.drawPixmap(m_x, m_y, m_bulletImage);
    } else {
        if (m_explodeFrame < m_bulletBoomImages.size()) {
            painter.drawPixmap(m_x, m_y, m_bulletBoomImages[m_explodeFrame]);
            ++m_explodeFrame;
        } else {
            m_explodeOver = true;
        }
    }
}

QSize Bullet::bulletSize() { return kBulletDrawSize; }
int Bullet::collisionSize() { return kBulletCollisionSize; }
