#include "tankbattle.h"
#include "game_map.h"
#include "bullet.h"
#include "player.h"
#include "enemy.h"

#include <QPainter>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QApplication>
#include <QTime>

static constexpr int TILE_SIZE = GameMap::TILE_SIZE;

TankBattle::TankBattle(QWidget* parent)
    : QWidget(parent) {
    setWindowTitle(QString::fromUtf8("坦克大战"));
    setFixedSize(MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE);

    m_levels = { QString::fromUtf8(":/maps/maps/map1.txt"), QString::fromUtf8(":/maps/maps/map2.txt") };

    m_menuBackground = QPixmap(QString::fromUtf8(":/menu/images/menu/menu.png"))
        .scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_titleFont.setFamily("SimHei");
    m_titleFont.setPointSize(32);
    m_uiFont.setFamily("SimHei");
    m_uiFont.setPointSize(16);

    connect(&m_timer, &QTimer::timeout, this, &TankBattle::onTick);
    m_timer.start(16); // ~60 FPS
}

TankBattle::~TankBattle() {
    // 清理资源
    qDeleteAll(m_playerBullets);
    qDeleteAll(m_enemyBullets);
    qDeleteAll(m_enemies);
    delete m_player;
    delete m_gameMap;
}

bool TankBattle::showMenu() {
    m_inMenu = true;
    // 简化：按 Enter 开始，Esc 退出。实际可做完整菜单导航。
    // 在事件循环中由 keyPressEvent 设置状态。
    return true; // 立即允许开始，通过 startGame 控制实际启用
}

void TankBattle::startGame() {
    m_inMenu = false;
    m_running = true;
    m_gameOver = false;
    m_win = false;
    m_levelWin = false;
    m_overlayStartMs = 0;

    // 加载关卡
    m_currentLevel = 0;
    m_enemySpawned = 0;

    delete m_gameMap; m_gameMap = nullptr;
    delete m_player;  m_player = nullptr;
    qDeleteAll(m_enemies); m_enemies.clear();
    qDeleteAll(m_playerBullets); m_playerBullets.clear();
    qDeleteAll(m_enemyBullets);  m_enemyBullets.clear();

    // 初始化当前关
    m_gameMap = new GameMap(MAP_WIDTH, MAP_HEIGHT, m_levels[m_currentLevel]);

    // 生成玩家与敌人（占位：需结合你的 PlayerTank/EnemyTank 构造）
    // 示例：PlayerTank(position), EnemyTank(position, level, tileSize)
    // 这里仅保留位置选择逻辑，实际创建请替换对应构造与成员访问
    auto playerSpawns = m_gameMap->getPossiblePosPlayer();
    if (!playerSpawns.isEmpty()) {
        QPoint p = playerSpawns[QRandomGenerator::global()->bounded(playerSpawns.size())];
        m_player = new PlayerTank(QPoint(p.x() * TILE_SIZE, p.y() * TILE_SIZE), 1, 2, 0, PlayerTank::Direction::Up);
    }

    QVector<int> enemyLevels{1, 2};
    auto enemySpawns = m_gameMap->getPossiblePosEnemy();
    for (int i = 0; i < 3 && !enemySpawns.isEmpty(); ++i) {
        QPoint e = enemySpawns[QRandomGenerator::global()->bounded(enemySpawns.size())];
        int lv = enemyLevels[QRandomGenerator::global()->bounded(enemyLevels.size())];
        m_enemies.append(new EnemyTank(QPoint(e.x() * TILE_SIZE, e.y() * TILE_SIZE), lv, TILE_SIZE));
        ++m_enemySpawned;
    }
}

void TankBattle::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    if (m_inMenu) {
        painter.drawPixmap(0, 0, m_menuBackground);
        painter.setFont(m_titleFont);
        // 渐变红到黑标题（简化为红色）
        painter.setPen(QColor(255, 0, 0));
        painter.drawText(width() / 2 - 100, 60, QString::fromUtf8("坦克大战"));
        painter.setFont(m_uiFont);
        painter.setPen(Qt::white);
        painter.drawText(width() / 2 - 120, height() - 60, QString::fromUtf8("按 Enter 开始 / Esc 退出"));
        return;
    }

    if (m_gameMap) {
        m_gameMap->draw(painter);
    }

    // 绘制玩家、子弹、敌人
    if (m_player) m_player->draw(painter);
    for (Bullet* b : m_playerBullets) {
        if (b) b->draw(painter);
    }
    for (Bullet* b : m_enemyBullets) {
        if (b) b->draw(painter);
    }
    for (EnemyTank* e : m_enemies) { if (e) e->draw(painter); }

    // UI（示例：生命/分数等需从 PlayerTank 读取）
    painter.setFont(m_uiFont);
    painter.setPen(QColor(255, 255, 0));
    if (m_player) {
        painter.drawText(width() - 220, 20, QString::fromUtf8("生命: ") + QString::number(m_player->getHealth()));
        painter.drawText(width() - 220, 40, QString::fromUtf8("分数: ") + QString::number(m_player->getScore()));
        painter.drawText(width() - 220, 60, QString::fromUtf8("玩家等级: ") + QString::number(m_player->getLevel()));
    } else {
        painter.drawText(width() - 220, 20, QString::fromUtf8("生命: --"));
        painter.drawText(width() - 220, 40, QString::fromUtf8("分数: --"));
        painter.drawText(width() - 220, 60, QString::fromUtf8("玩家等级: --"));
    }

    if (m_gameOver) {
        // 灰色蒙版
        painter.fillRect(rect(), QColor(0, 0, 0, 150));
        // 加粗放大字体
        QFont bigFont = m_titleFont; bigFont.setBold(true); bigFont.setPointSize(42);
        painter.setFont(bigFont);
        painter.setPen(m_win ? QColor(0, 255, 0) : QColor(255, 0, 0));
        QString msg;
        if (m_win && m_currentLevel < m_levels.size() - 1 && m_levelWin)
            msg = QString::fromUtf8("胜利！");
        else if (m_win && m_currentLevel >= m_levels.size() - 1)
            msg = QString::fromUtf8("通关！");
        else
            msg = QString::fromUtf8("失败");
        painter.drawText(width() / 2 - 100, height() / 2, msg);
    }
}

void TankBattle::keyPressEvent(QKeyEvent* e) {
    if (m_inMenu) {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            startGame();
        } else if (e->key() == Qt::Key_Escape) {
            qApp->quit();
        }
        return;
    }

    if (e->key() == Qt::Key_W || e->key() == Qt::Key_Up) m_pressUp = true;
    if (e->key() == Qt::Key_S || e->key() == Qt::Key_Down) m_pressDown = true;
    if (e->key() == Qt::Key_A || e->key() == Qt::Key_Left) m_pressLeft = true;
    if (e->key() == Qt::Key_D || e->key() == Qt::Key_Right) m_pressRight = true;
    if (e->key() == Qt::Key_Space) m_pressShoot = true;
}

void TankBattle::keyReleaseEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_W || e->key() == Qt::Key_Up) m_pressUp = false;
    if (e->key() == Qt::Key_S || e->key() == Qt::Key_Down) m_pressDown = false;
    if (e->key() == Qt::Key_A || e->key() == Qt::Key_Left) m_pressLeft = false;
    if (e->key() == Qt::Key_D || e->key() == Qt::Key_Right) m_pressRight = false;
    if (e->key() == Qt::Key_Space) m_pressShoot = false;
}

void TankBattle::onTick() {
    // 菜单时不推进游戏逻辑；非运行但正在显示结算蒙版时仍需计时
    if (m_inMenu) { update(); return; }
    if (!m_running && !m_gameOver) { update(); return; }

    // 玩家输入
    if (m_player) {
        if (m_pressUp)      m_player->move(PlayerTank::Direction::Up,    m_gameMap);
        else if (m_pressDown)  m_player->move(PlayerTank::Direction::Down,  m_gameMap);
        else if (m_pressLeft)  m_player->move(PlayerTank::Direction::Left,  m_gameMap);
        else if (m_pressRight) m_player->move(PlayerTank::Direction::Right, m_gameMap);
        if (m_pressShoot) {
            int now = QTime::currentTime().msecsSinceStartOfDay();
            // 限制玩家射速：每 200ms 一发
            if (now - m_player->lastFireTimeMs() >= 200) {
                Bullet* b = m_player->shoot();
                if (b) m_playerBullets.append(b);
            }
        }
    }

    // 敌人 AI
    for (EnemyTank* e : m_enemies) {
        if (!e) continue;
        QPoint playerPos = m_player ? m_player->position() : QPoint(0,0);
        e->move(m_player ? &playerPos : nullptr, m_gameMap);
        if (Bullet* eb = e->shoot()) {
            m_enemyBullets.append(eb);
        }
    }

    // 动态生成敌人（与 Python 逻辑一致）
    int aliveCount = 0;
    for (EnemyTank* e : m_enemies) { if (e && !e->blasted()) ++aliveCount; }
    if (m_enemySpawned < m_enemyTotal && aliveCount < 3) {
        auto enemySpawns = m_gameMap->getPossiblePosEnemy();
        // 过滤不可用出生点（与玩家/敌人重叠，或爆炸中）
        QVector<QPoint> freeSpawns;
        for (const QPoint& gp : enemySpawns) {
            if (isSpawnFree(gp)) freeSpawns.append(gp);
        }
        if (!freeSpawns.isEmpty()) {
            QPoint epos = freeSpawns[QRandomGenerator::global()->bounded(freeSpawns.size())];
            int lv = (QRandomGenerator::global()->bounded(2) == 0) ? 1 : 2;
            m_enemies.append(new EnemyTank(QPoint(epos.x() * TILE_SIZE, epos.y() * TILE_SIZE), lv, TILE_SIZE));
            ++m_enemySpawned;
        }
        // 若没有可用出生点，则本帧不生成，等待下一帧重试
    }

    // 统一处理子弹
    handleBullets(m_playerBullets, true);
    handleBullets(m_enemyBullets, false);

    // 胜利/失败判定
    bool allDead = true; for (EnemyTank* e : m_enemies) { if (e && !e->blasted()) { allDead = false; break; } }
    if (allDead && m_enemySpawned >= m_enemyTotal) {
        // 胜利：仅当不是最后一关时才视为“小关胜利”，否则进入通关结算
        m_gameOver = true;
        m_win = true;
        m_levelWin = (m_currentLevel < m_levels.size() - 1);
        if (m_overlayStartMs == 0) m_overlayStartMs = QTime::currentTime().msecsSinceStartOfDay();
    }
    if (m_gameMap && m_gameMap->isBaseDestroyed()) { m_gameOver = true; m_win = false; }

    if (m_gameOver) {
        int now = QTime::currentTime().msecsSinceStartOfDay();
        if (m_win && m_levelWin) {
            // 小关胜利：3秒后进入下一关
            if (m_overlayStartMs == 0) m_overlayStartMs = now;
            if (now - m_overlayStartMs >= 3000) {
                m_levelWin = false; m_gameOver = false;
                nextLevelOrEnd(true);
                m_overlayStartMs = 0;
            }
        } else {
            // 整体失败或最终通关：5秒后返回菜单
            if (m_overlayStartMs == 0) m_overlayStartMs = now;
            if (now - m_overlayStartMs >= 5000) {
                resetToMenu();
            }
        }
    }

    update();
}

void TankBattle::handleBullets(QVector<Bullet*>& bullets, bool isPlayer) {
    // 推进子弹，并处理简单碰撞：地图、敌人/玩家
    for (int i = bullets.size() - 1; i >= 0; --i) {
        Bullet* b = bullets[i];
        if (!b) { bullets.remove(i); continue; }
        b->move();
        // 碰撞：地图边界或墙
        QRect bRect(b->x(), b->y(), Bullet::bulletSize().width(), Bullet::bulletSize().height());
        bool hitWall = false;
        if (!rect().contains(bRect)) {
            hitWall = true;
        } else if (m_gameMap && m_gameMap->checkCollisionBullet(bRect)) {
            hitWall = true;
            // 计算子弹命中格子，并尝试摧毁可破坏砖块（假设 tile==1 为砖块）
            int gx = (b->x() + Bullet::bulletSize().width() / 2) / TILE_SIZE;
            int gy = (b->y() + Bullet::bulletSize().height() / 2) / TILE_SIZE;
            int tile = m_gameMap->getTile(gx, gy);
            if (tile == 1) {
                m_gameMap->destroyTile(gx, gy);
            } else if (tile == 5) { // 假设 5 为基地
                m_gameMap->baseDestroyed();
                m_gameOver = true; m_win = false;
            }
        }
        if (hitWall) {
            b->boom();
        }
        // 碰撞：敌人或玩家
        if (isPlayer) {
            for (EnemyTank* e : m_enemies) {
                if (!e || e->blasted()) continue;
                // 使用更紧凑的碰撞箱来提高命中判定准确性
                int cs = Bullet::collisionSize();
                QRect eRect(e->x() + (TILE_SIZE - cs)/2, e->y() + (TILE_SIZE - cs)/2, cs, cs);
                if (b->checkCollision(eRect)) {
                    b->boom();
                    // 玩家子弹命中敌人：削减敌人生命值，归零则进入爆炸状态
                    e->takeDamage(1);
                    e->onHitFeedback();
                    if (e->blasted()) {
                        m_player->increaseScore(1);
                    }
                    break;
                }
            }
        } else {
            if (m_player) {
                QRect pRect(m_player->x(), m_player->y(), TILE_SIZE, TILE_SIZE);
                if (b->checkCollision(pRect)) {
                    b->boom();
                    bool dead = m_player->takeDamage(1);
                    if (dead) {
                        m_gameOver = true; m_win = false;
                    }
                }
            }
        }
        if (b->exploded() && b->explodeOver()) {
            delete b;
            bullets.remove(i);
        }
    }
}

void TankBattle::nextLevelOrEnd(bool win) {
    if (!win) { m_running = false; return; }
    if (m_currentLevel + 1 < m_levels.size()) {
        ++m_currentLevel;
        // 重新加载下一关
        delete m_gameMap; m_gameMap = new GameMap(MAP_WIDTH, MAP_HEIGHT, m_levels[m_currentLevel]);
        qDeleteAll(m_enemies); m_enemies.clear();
        m_enemySpawned = 0;
        // 重新生成初始敌人
        auto enemySpawns = m_gameMap->getPossiblePosEnemy();
        QVector<int> enemyLevels{1, 2};
        for (int i = 0; i < 3 && !enemySpawns.isEmpty(); ++i) {
            QPoint e = enemySpawns[QRandomGenerator::global()->bounded(enemySpawns.size())];
            int lv = enemyLevels[QRandomGenerator::global()->bounded(enemyLevels.size())];
            m_enemies.append(new EnemyTank(QPoint(e.x() * TILE_SIZE, e.y() * TILE_SIZE), lv, TILE_SIZE));
            ++m_enemySpawned;
        }
        m_gameOver = false;
        // 重新生成玩家
        auto playerSpawns = m_gameMap->getPossiblePosPlayer();
        delete m_player; m_player = nullptr;
        if (!playerSpawns.isEmpty()) {
            QPoint p = playerSpawns[QRandomGenerator::global()->bounded(playerSpawns.size())];
            m_player = new PlayerTank(QPoint(p.x() * TILE_SIZE, p.y() * TILE_SIZE), 1, 2, 0, PlayerTank::Direction::Up);
        }
        // 清空子弹
        qDeleteAll(m_playerBullets); m_playerBullets.clear();
        qDeleteAll(m_enemyBullets); m_enemyBullets.clear();
    } else {
        m_running = false; // 最后一关结束，等待 resetToMenu()
    }
}

bool TankBattle::isSpawnFree(const QPoint& gridPos) const {
    QRect spawnRect(gridPos.x() * TILE_SIZE, gridPos.y() * TILE_SIZE, TILE_SIZE, TILE_SIZE);
    // 不与玩家重叠
    if (m_player) {
        QRect pRect(m_player->x(), m_player->y(), TILE_SIZE, TILE_SIZE);
        if (spawnRect.intersects(pRect)) return false;
    }
    // 不与任一敌人重叠（存活或爆炸中）
    for (EnemyTank* e : m_enemies) {
        if (!e) continue;
        QRect eRect(e->x(), e->y(), TILE_SIZE, TILE_SIZE);
        // 爆炸中也视为占位，避免“原地重生”观感
        if (!e->blasted() || (e->blasted() && !e->blastOver())) {
            if (spawnRect.intersects(eRect)) return false;
        }
    }
    return true;
}

void TankBattle::resetToMenu() {
    // 返回菜单并重置对象
    m_inMenu = true;
    m_running = false;
    m_gameOver = false;
    m_win = false;
    m_levelWin = false;
    m_overlayStartMs = 0;
    delete m_gameMap; m_gameMap = nullptr;
    delete m_player;  m_player = nullptr;
    qDeleteAll(m_enemies); m_enemies.clear();
    qDeleteAll(m_playerBullets); m_playerBullets.clear();
    qDeleteAll(m_enemyBullets);  m_enemyBullets.clear();
    m_currentLevel = 0;
    update();
}
