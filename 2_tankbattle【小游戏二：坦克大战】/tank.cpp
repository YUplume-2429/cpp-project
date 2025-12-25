#include "tank.h"
#include "bullet.h"
#include "game_map.h"
#include <QTime>

namespace {
    constexpr int BULLET_BASE_SPEED = 5;
    constexpr int TANK_BASE_SPEED   = 2;
    // 与 Python 的 res_dir 一致，使用 TILE_SIZE * 0.5 偏移
    QPoint bulletOffset(Tank::Direction d) {
        int off = GameMap::TILE_SIZE / 2; // 近似 0.5 * TILE_SIZE
        switch (d) {
        case Tank::Direction::Up:    return QPoint(off, 0);
        case Tank::Direction::Down:  return QPoint(off, 0);
        case Tank::Direction::Left:  return QPoint(0, off);
        case Tank::Direction::Right: return QPoint(0, off);
        }
        return QPoint(0, 0);
    }

    int ticks() { return QTime::currentTime().msecsSinceStartOfDay(); }
}

Tank::Tank(int level)
    : m_level(level > 0 ? level : 1) {
    m_maxHealth = m_level;
    m_health = m_maxHealth;
    m_speed = m_level * TANK_BASE_SPEED;
    m_bulletSpeed = m_level;
    loadBlastImages();
}

void Tank::loadBlastImages() {
    const char* paths[] = {
        ":/boom/images/boom/blast1.gif",
        ":/boom/images/boom/blast2.gif",
        ":/boom/images/boom/blast3.gif",
        ":/boom/images/boom/blast4.gif",
        ":/boom/images/boom/blast5.gif",
        ":/boom/images/boom/blast6.gif",
        ":/boom/images/boom/blast7.gif",
        ":/boom/images/boom/blast8.gif",
    };
    for (const char* p : paths) {
        QPixmap pm(QString::fromUtf8(p));
        m_boomImages.push_back(pm);
    }
    m_blastMaxFrame = m_boomImages.size() - 1;
}

void Tank::move() {
    switch (m_direction) {
    case Direction::Up:    m_y -= m_speed; break;
    case Direction::Down:  m_y += m_speed; break;
    case Direction::Left:  m_x -= m_speed; break;
    case Direction::Right: m_x += m_speed; break;
    }
}

void Tank::takeDamage(int amount) {
    m_health -= amount;
    if (m_health <= 0) {
        die();
    }
}

void Tank::drawBlast(QPainter& painter) {
    if (m_blasted && !m_blastOver && !m_boomImages.isEmpty()) {
        int idx = m_blastFrame;
        if (idx >= 0 && idx < m_boomImages.size()) {
            painter.drawPixmap(m_x, m_y, m_boomImages[idx]);
        }
        ++m_blastFrame;
        if (m_blastFrame > m_blastMaxFrame) {
            m_blasted = false;
            m_blastFrame = 0;
            m_blastOver = true;
        }
    }
}

void Tank::playBlastSound() {
    // 占位：可用 QSoundEffect 或 QMediaPlayer 实现
}

void Tank::die() {
    m_blasted = true;
    playBlastSound();
}

void Tank::playFireSound() {
    // 占位：可用 QSoundEffect 或 QMediaPlayer 实现
}

Bullet* Tank::shoot() {
    int now = ticks();
    if (now - m_lastFireTimeMs > m_fireIntervalMs) {
        playFireSound();
        m_lastFireTimeMs = now;
        QPoint nowPos(m_x, m_y);
        QPoint off = bulletOffset(m_direction);
        QPoint bulletPos = nowPos + off;
        int speed = m_bulletSpeed * BULLET_BASE_SPEED;

        Bullet::Direction dir = Bullet::Direction::Up;
        switch (m_direction) {
        case Direction::Up:    dir = Bullet::Direction::Up; break;
        case Direction::Down:  dir = Bullet::Direction::Down; break;
        case Direction::Left:  dir = Bullet::Direction::Left; break;
        case Direction::Right: dir = Bullet::Direction::Right; break;
        }
        return new Bullet(bulletPos, dir, speed);
    }
    return nullptr;
}
