#ifndef TANKBATTLE_H
#define TANKBATTLE_H

#include <QWidget>
#include <QPixmap>
#include <QTimer>
#include <QVector>
#include <QPoint>
#include <QString>
#include <QFont>

class GameMap;
class EnemyTank;
class PlayerTank; // 需按你的项目提供 Qt 版本
class Bullet;     // 已在 bullet.h 中定义

class TankBattle : public QWidget {
    Q_OBJECT
public:
    explicit TankBattle(QWidget* parent = nullptr);
    ~TankBattle() override;

    bool showMenu();
    void startGame();

protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;

private slots:
    void onTick();

private:
    void handleBullets(QVector<Bullet*>& bullets, bool isPlayer);
    void nextLevelOrEnd(bool win);
    bool isSpawnFree(const QPoint& gridPos) const; // 检查该网格是否可安全生成敌人

private:
    // 常量与尺寸
    static constexpr int MAP_WIDTH = 20;
    static constexpr int MAP_HEIGHT = 15;

    // 资源
    QPixmap m_menuBackground;
    QFont   m_titleFont;
    QFont   m_uiFont;

    // 游戏状态
    bool m_running{false};
    bool m_inMenu{true};
    bool m_gameOver{false};
    bool m_win{false};
    // 结束/胜利蒙版与计时
    int m_overlayStartMs{0};
    bool m_levelWin{false}; // 单关胜利，用于3秒后进入下一关

    // 地图与角色
    GameMap* m_gameMap{nullptr};
    PlayerTank* m_player{nullptr};
    QVector<EnemyTank*> m_enemies;
    QVector<Bullet*> m_playerBullets;
    QVector<Bullet*> m_enemyBullets;

    // 关卡
    QVector<QString> m_levels;
    int m_currentLevel{0};
    int m_enemyTotal{6};
    int m_enemySpawned{0};

    // 输入状态
    bool m_pressUp{false};
    bool m_pressDown{false};
    bool m_pressLeft{false};
    bool m_pressRight{false};
    bool m_pressShoot{false};

    // 时钟
    QTimer m_timer;

    // 辅助：重置到菜单
    void resetToMenu();
};

#endif // TANKBATTLE_H
