#include "enemy.h"
#include "game_map.h" // 需要完整类型以调用成员函数
#include <QRandomGenerator>
#include <QTime>
#include <QPainter>
#include <algorithm>

// 依赖接口假定：
// class GameMap { public: int getTile(int gx, int gy) const; bool checkCollision(const QRect& r) const; };
// class Tank   { public:
//   void drawBlast(QPainter& painter, int x, int y, int& explodeFrame, int& blastMaxFrame, bool& blastOver);
//   void shoot();
// };

static int currentTicks() {
    // 使用 Qt 时间获取毫秒
    return QTime::currentTime().msecsSinceStartOfDay();
}

static QString enemyPath(int level, char dir) {
    // 与 Python 路径一致
    // level: 1/2, dir: 'D','L','R','U'
    return QString(":/enemy/images/enemy/enemy%1%2.gif").arg(level).arg(QChar(dir));
}

EnemyTank::EnemyTank(const QPoint& position, int level, int tileSize)
    : Tank(level), m_tileSize(tileSize) {
    setPosition(position);
    // 加载四方向贴图
    m_enemyD = QPixmap(enemyPath(level, 'D'));
    m_enemyL = QPixmap(enemyPath(level, 'L'));
    m_enemyR = QPixmap(enemyPath(level, 'R'));
    m_enemyU = QPixmap(enemyPath(level, 'U'));

    // 存弹上限：level * 2
    m_maxStoreBullets = std::max(0, level * 2);
}

const QPixmap& EnemyTank::spriteFor(Direction d) const {
    switch (d) {
    case Direction::Down: return m_enemyD;
    case Direction::Left: return m_enemyL;
    case Direction::Right: return m_enemyR;
    case Direction::Up: default: return m_enemyU;
    }
}

bool EnemyTank::detectPlayer(const QPoint& playerPosition, const GameMap* gameMap) const {
    if (!gameMap) return false;
    // 玩家在禁区（tile==3）不追踪
    int gx0 = x() / m_tileSize;
    int gy0 = y() / m_tileSize;
    int gx1 = playerPosition.x() / m_tileSize;
    int gy1 = playerPosition.y() / m_tileSize;
    int dist = std::abs(gx0 - gx1) + std::abs(gy0 - gy1);

    if (gameMap->getTile(gx1, gy1) == 3) return false;
    if (dist > kDetectRange) return false;

    // 直线视野（同列）
    if (gx0 == gx1) {
        int step = (gy1 > gy0) ? 1 : -1;
        for (int yy = gy0 + step; yy != gy1; yy += step) {
            int t = gameMap->getTile(gx0, yy);
            if (t == 1 || t == 2 || t == 3 || t == 5) return false;
        }
        return true;
    }
    // 同行
    if (gy0 == gy1) {
        int step = (gx1 > gx0) ? 1 : -1;
        for (int xx = gx0 + step; xx != gx1; xx += step) {
            int t = gameMap->getTile(xx, gy0);
            if (t == 1 || t == 2 || t == 3 || t == 5) return false;
        }
        return true;
    }

    // 近距离触发
    return dist <= kDetectRange;
}

void EnemyTank::move(const QPoint* playerPosition, const GameMap* gameMap) {
    if (m_moving || blasted()) return;

    if (playerPosition && gameMap && detectPlayer(*playerPosition, gameMap)) {
        m_behavior = Behavior::Aggressive;
    } else {
        m_behavior = Behavior::Patrol;
    }

    if (m_behavior == Behavior::Patrol) {
        patrol(gameMap);
    } else if (playerPosition) {
        chasePlayer(*playerPosition, gameMap);
    }
}

void EnemyTank::patrol(const GameMap* gameMap) {
    // 2% 概率随机挑选方向
    if (QRandomGenerator::global()->bounded(101) < 2) {
        int r = QRandomGenerator::global()->bounded(4);
        setDirection(static_cast<Direction>(r));
    }
    tryMove(gameMap);
}

void EnemyTank::chasePlayer(const QPoint& playerPosition, const GameMap* gameMap) {
    if (blasted()) return;
    int px = playerPosition.x();
    int py = playerPosition.y();
    int cx = x();
    int cy = y();
    if (std::abs(cx - px) > std::abs(cy - py)) {
        setDirection((cx < px) ? Direction::Right : Direction::Left);
    } else {
        setDirection((cy < py) ? Direction::Down : Direction::Up);
    }
    tryMove(gameMap);
}

void EnemyTank::tryMove(const GameMap* gameMap) {
    if (blasted()) return;

    int old_x = x(), old_y = y();
    int tx = old_x, ty = old_y;
    switch (direction()) {
    case Direction::Up:    ty -= m_tileSize; break;
    case Direction::Down:  ty += m_tileSize; break;
    case Direction::Left:  tx -= m_tileSize; break;
    case Direction::Right: tx += m_tileSize; break;
    }

    QRect testRect(tx, ty, m_tileSize, m_tileSize);
    bool turnChance = (QRandomGenerator::global()->bounded(101) >= 97); // 3/100 的概率尝试转向
    if ((gameMap && gameMap->checkCollisionTank(testRect)) || turnChance) {
        // 回退并尝试其它方向
        QVector<Direction> dirs{Direction::Up, Direction::Down, Direction::Left, Direction::Right};
        // 移除当前方向
        dirs.erase(std::remove(dirs.begin(), dirs.end(), direction()), dirs.end());

        // 随机排列
        std::shuffle(dirs.begin(), dirs.end(), *QRandomGenerator::global());

        bool found = false;
        for (Direction d : dirs) {
            int nx = old_x, ny = old_y;
            switch (d) {
            case Direction::Up:    ny -= m_tileSize; break;
            case Direction::Down:  ny += m_tileSize; break;
            case Direction::Left:  nx -= m_tileSize; break;
            case Direction::Right: nx += m_tileSize; break;
            }
            QRect r(nx, ny, m_tileSize, m_tileSize);
            if (!gameMap || !gameMap->checkCollisionTank(r)) {
                setDirection(d);
                tx = nx; ty = ny;
                found = true;
                break;
            }
        }
        if (!found) return; // 全部方向被堵死，放弃移动
    }

    // 开始线性插值移动
    m_moveStartPos = QPoint(old_x, old_y);
    m_moveTarget   = QPoint(tx, ty);
    m_moveStartTime = currentTicks();
    m_moving = true;
}

Bullet* EnemyTank::shoot() {
    if (blasted()) return nullptr;

    // 随机触发与存弹逻辑
    if (QRandomGenerator::global()->bounded(301) < 2) {
        if (QRandomGenerator::global()->bounded(11) < 7) {
            if (m_storedBullet < m_maxStoreBullets) {
                ++m_storedBullet;
            }
        } else {
            // 敌人射速节流
            int now = currentTicks();
            if (now - m_enemyLastShootMs >= m_enemyFireIntervalMs) {
                m_enemyLastShootMs = now;
                return Tank::shoot();
            }
            return nullptr;
        }
    }

    if (m_behavior == Behavior::Aggressive) {
        if (m_storedBullet > 0) {
            --m_storedBullet;
            int now = currentTicks();
            if (now - m_enemyLastShootMs >= m_enemyFireIntervalMs) {
                m_enemyLastShootMs = now;
                return Tank::shoot();
            }
            return nullptr;
        } else {
            if (QRandomGenerator::global()->bounded(301) < 5) {
                int now = currentTicks();
                if (now - m_enemyLastShootMs >= m_enemyFireIntervalMs) {
                    m_enemyLastShootMs = now;
                    return Tank::shoot();
                }
            }
        }
    }
    return nullptr;
}

void EnemyTank::takeDamage(int amount) {
    Tank::takeDamage(amount);
}

void EnemyTank::die() {
    Tank::die();
}

void EnemyTank::update() {
    // 爆炸帧推进由 Tank::drawBlast 管理，此处无需额外推进

    if (m_hitFlashTicks > 0) {
        --m_hitFlashTicks;
    }

    if (m_moving) {
        int now = currentTicks();
        int elapsed = now - m_moveStartTime;
        double t = std::min(1.0, elapsed / static_cast<double>(m_moveDurationMs));
        int sx = m_moveStartPos.x();
        int sy = m_moveStartPos.y();
        int tx = m_moveTarget.x();
        int ty = m_moveTarget.y();
        int nx = static_cast<int>(sx + (tx - sx) * t);
        int ny = static_cast<int>(sy + (ty - sy) * t);
        setPosition(QPoint(nx, ny));
        if (t >= 1.0) {
            setPosition(QPoint(tx, ty));
            m_moving = false;
        }
    }
}

void EnemyTank::draw(QPainter& painter) {
    update();

    // 爆炸绘制由 Tank 接口处理（此处保留占位以便集成）
    // 假设 asTankInterface.drawBlast 会修改爆炸帧与结束标志
    // asTankInterface.drawBlast(painter, m_x, m_y, m_explodeFrame, m_blastMaxFrame, m_blastOver);

    if (!blasted()) {
        if (m_hitFlashTicks > 0) {
            // 受击时简单叠加半透明红色方块作为反馈
            painter.drawPixmap(x(), y(), spriteFor(direction()));
            painter.fillRect(QRect(x(), y(), m_tileSize, m_tileSize), QColor(255, 0, 0, 80));
        } else {
            painter.drawPixmap(x(), y(), spriteFor(direction()));
        }
    }
    Tank::drawBlast(painter);
}

void EnemyTank::onHitFeedback() {
    // 命中后触发短暂闪烁
    m_hitFlashTicks = 6; // ~6 帧（约 100ms）
}
