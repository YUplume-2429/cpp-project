#include "game_map.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QRect>

// 资源路径与 Python 保持一致
static const char* kWallPaths[6] = {
    ":/walls/images/walls/0.png",
    ":/walls/images/walls/1.png",
    ":/walls/images/walls/2.png",
    ":/walls/images/walls/3.png",
    ":/walls/images/walls/4.png",
    ":/walls/images/walls/5.png",
};

GameMap::GameMap(int width, int height, const QString& mapFile)
    : m_width(width), m_height(height) {
    // 加载贴图
    for (int i = 0; i < 6; ++i) {
        m_wallImgs[i] = QPixmap(QString::fromUtf8(kWallPaths[i]));
    }
    // 读取地图
    m_mapData = loadMap(mapFile);
}

QVector<QVector<int>> GameMap::loadMap(const QString& mapFile) const {
    QVector<QVector<int>> data;
    QFile f(mapFile);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return data;
    }
    QTextStream in(&f);
    QString content = in.readAll();
    f.close();

    // Python 用的是 eval(list)，这里简易解析：
    // 期望格式类似 [[0,1,...],[...],...]
    // 为避免引入脚本解析，这里做一个非常基础的解析器。
    QString s = content;
    s.remove(' ');
    s.remove('\n');
    // 去掉外层 []
    if (s.startsWith('[') && s.endsWith(']')) {
        s = s.mid(1, s.size() - 2);
    }
    // 按行分割：通过 '],[' 标识
    QStringList rows = s.split("],[", Qt::SkipEmptyParts);
    for (int ri = 0; ri < rows.size(); ++ri) {
        QString row = rows[ri];
        if (row.startsWith('[')) row = row.mid(1);
        if (row.endsWith(']')) row = row.left(row.size() - 1);
        QStringList nums = row.split(',', Qt::SkipEmptyParts);
        QVector<int> r;
        r.reserve(nums.size());
        for (const QString& n : nums) {
            bool ok = false;
            int v = n.toInt(&ok);
            r.push_back(ok ? v : 0);
        }
        data.push_back(r);
    }
    return data;
}

void GameMap::draw(QPainter& painter) const {
    for (int y = 0; y < m_mapData.size(); ++y) {
        const auto& row = m_mapData[y];
        for (int x = 0; x < row.size(); ++x) {
            int tile = row[x];
            int px = x * TILE_SIZE;
            int py = y * TILE_SIZE;
            if (tile >= 0 && tile <= 5) {
                painter.drawPixmap(px, py, m_wallImgs[tile]);
            }
        }
    }
}

bool GameMap::checkCollisionTank(const QRect& rect) const {
    if (rect.left() < 0 || rect.right() > m_width * TILE_SIZE || rect.top() < 0 || rect.bottom() > m_height * TILE_SIZE) {
        return true;
    }
    // 收缩坦克碰撞箱每边3px
    QRect tankRect = rect.adjusted(3, 3, -3, -3);
    // 阻挡：红砖(1)、铁墙(2)、河流(4)、司令部(5)
    for (int y = 0; y < m_mapData.size(); ++y) {
        const auto& row = m_mapData[y];
        for (int x = 0; x < row.size(); ++x) {
            int tile = row[x];
            if (tile == 1 || tile == 2 || tile == 4 || tile == 5) {
                QRect tileRect(x * TILE_SIZE + 2, y * TILE_SIZE + 2, TILE_SIZE - 4, TILE_SIZE - 4);
                if (tileRect.intersects(tankRect)) return true;
            }
        }
    }
    return false;
}

bool GameMap::checkCollisionBullet(const QRect& rect) const {
    if (rect.left() < 0 || rect.right() > m_width * TILE_SIZE || rect.top() < 0 || rect.bottom() > m_height * TILE_SIZE) {
        return true;
    }
    // 阻挡：红砖(1)、铁墙(2)、司令部(5)；河流(4)不阻挡子弹
    for (int y = 0; y < m_mapData.size(); ++y) {
        const auto& row = m_mapData[y];
        for (int x = 0; x < row.size(); ++x) {
            int tile = row[x];
            if (tile == 1 || tile == 2 || tile == 5) {
                QRect tileRect(x * TILE_SIZE + 2, y * TILE_SIZE + 2, TILE_SIZE - 4, TILE_SIZE - 4);
                if (tileRect.intersects(rect)) return true;
            }
        }
    }
    return false;
}

void GameMap::baseDestroyed() {
    m_baseDestroyed = true;
}

void GameMap::destroyTile(int x, int y) {
    if (y < 0 || y >= m_mapData.size()) return;
    if (x < 0 || x >= m_mapData[y].size()) return;
    int code = m_mapData[y][x];
    if (code == 1 || code == 5) {
        m_mapData[y][x] = 0;
        if (code == 5) baseDestroyed();
    }
}

int GameMap::getTile(int x, int y) const {
    if (y < 0 || y >= m_mapData.size()) return 0;
    if (x < 0 || x >= m_mapData[y].size()) return 0;
    return m_mapData[y][x];
}

QVector<QPoint> GameMap::getPossiblePos() const {
    QVector<QPoint> positions;
    for (int y = 0; y < m_mapData.size(); ++y) {
        const auto& row = m_mapData[y];
        for (int x = 0; x < row.size(); ++x) {
            int tile = row[x];
            if (tile == 0 || tile == 3) {
                positions.append(QPoint(x, y));
            }
        }
    }
    return positions;
}

QVector<QPoint> GameMap::getPossiblePosEnemy() const {
    QVector<QPoint> result;
    QVector<QPoint> base = getPossiblePos();
    for (const QPoint& p : base) {
        if (p.y() < m_mapData.size() / 3) {
            result.append(p);
        }
    }
    return result;
}

QVector<QPoint> GameMap::getPossiblePosPlayer() const {
    QVector<QPoint> result;
    QVector<QPoint> base = getPossiblePos();
    for (const QPoint& p : base) {
        if (p.y() >= (m_mapData.size() * 2) / 3) {
            if (getTile(p.x(), p.y()) == 3) {
                result.append(p);
            }
        }
    }
    return result;
}
