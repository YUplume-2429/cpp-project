#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <QPixmap>
#include <QVector>
#include <QRect>
#include <QPoint>
#include <QString>
#include <QPainter>

class GameMap {
public:
    static constexpr int TILE_SIZE = 40; // 与 Python 常量一致

    GameMap(int width, int height, const QString& mapFile);

    void draw(QPainter& painter) const;
    // 坦克碰撞：边界+阻挡（含河流）并对坦克碰撞箱收缩3px
    bool checkCollisionTank(const QRect& rect) const;
    // 子弹碰撞：边界+阻挡（不阻挡河流，子弹可通过）
    bool checkCollisionBullet(const QRect& rect) const;

    void baseDestroyed();
    bool isBaseDestroyed() const { return m_baseDestroyed; }
    void destroyTile(int x, int y);

    int getTile(int x, int y) const;
    QVector<QPoint> getPossiblePos() const;
    QVector<QPoint> getPossiblePosEnemy() const;
    QVector<QPoint> getPossiblePosPlayer() const;

    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    QVector<QVector<int>> loadMap(const QString& mapFile) const;

private:
    int m_width{0};
    int m_height{0};
    QVector<QVector<int>> m_mapData;

    // 0..5 的墙体贴图
    QPixmap m_wallImgs[6];

    bool m_baseDestroyed{false};
};

#endif // GAME_MAP_H
