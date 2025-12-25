#include "player.h"
#include "bullet.h"
#include "game_map.h"
#include <QTime>

static QString heroPath(char dir) {
    // 使用 Qt 资源前缀与现有资源结构一致
    return QString(":/hero/images/hero/hero1%1.gif").arg(QChar(dir));
}

PlayerTank::PlayerTank(const QPoint& position, int level, int lives, int score, Direction direction)
    : Tank(level), m_direction(direction), m_level(level), m_lives(lives), m_score(score) {
    setPosition(position);
    // health 按等级设定（示例：3*level，可与 Tank 的 maxHealth 协调）
    m_health = 3 * m_level;
    reloadImages();
}

void PlayerTank::reloadImages() {
    m_heroD = QPixmap(heroPath('D'));
    m_heroL = QPixmap(heroPath('L'));
    m_heroR = QPixmap(heroPath('R'));
    m_heroU = QPixmap(heroPath('U'));
}

void PlayerTank::move(Direction direction, const GameMap* gameMap) {
    if (m_blasted || !m_alive) return;
    if (direction != m_direction) setDirection(direction);
    m_direction = direction;

    int old_x = x(), old_y = y();
    Tank::move();

    if (gameMap) {
        QRect rect(x(), y(), GameMap::TILE_SIZE, GameMap::TILE_SIZE);
        if (gameMap->checkCollisionTank(rect)) {
            setPosition(QPoint(old_x, old_y));
        }
    }
}

Bullet* PlayerTank::shoot() {
    if (m_blasted || !m_alive) return nullptr;
    // 使用基类射击逻辑，包含发射间隔与速度等
    return Tank::shoot();
}

void PlayerTank::increaseScore(int points) {
    m_score += points;
    m_killCount += points;
    // 连续击败2辆敌人升级，最高3级
    if (m_level < 3 && m_killCount >= 2) {
        ++m_level;
        // 重新初始化保留位置、生命值、分数、方向
        Direction dir = m_direction;
        int lives = m_lives;
        int score = m_score;
        QPoint pos = position();
        *this = PlayerTank(pos, m_level, lives, score, dir);
    }
}


bool PlayerTank::takeDamage(int dmg) {
    if (!m_alive) return false;
    m_health -= dmg;
    if (m_health <= 0) {
        // die()
        m_blasted = true;
        m_explodeFrame = 0;
        return true;
    }
    return false;
}

void PlayerTank::dieOnce(const QPoint& position) {
    // super().die()
    m_blasted = true;
    m_explodeFrame = 0;
    // 重生
    int lives = m_lives - 1;
    m_lives = lives;
    *this = PlayerTank(position, m_level, m_lives, m_score, m_direction);
    if (m_lives <= 0) {
        m_alive = false;
    }
}

void PlayerTank::reset(const QPoint& position) {
    *this = PlayerTank(position, m_level, m_lives, m_score, m_direction);
}

void PlayerTank::draw(QPainter& painter) {
    if (!m_blasted) {
        const QPixmap* pm = nullptr;
        switch (m_direction) {
        case Direction::Down: pm = &m_heroD; break;
        case Direction::Left: pm = &m_heroL; break;
        case Direction::Right: pm = &m_heroR; break;
        case Direction::Up: default: pm = &m_heroU; break;
        }
        if (pm) painter.drawPixmap(x(), y(), *pm);
    }
    // 使用 Tank 的爆炸绘制方法
    Tank::drawBlast(painter);
}

// 保留：爆炸状态变量由 Tank 管理帧绘制，此处不再重复实现
