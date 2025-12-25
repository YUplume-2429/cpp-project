#include "boardwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QRandomGenerator>
#include <QTimer>
#include <QMessageBox>
#include <cmath>

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent)
{
    resetBoard();
    setAutoFillBackground(true);
}

QSize BoardWidget::minimumSizeHint() const { return QSize(600, 600); }
QSize BoardWidget::sizeHint() const { return QSize(700, 700); }

void BoardWidget::resetBoard()
{
    board_.clear();
    board_.reserve(kSize);
    for (int y = 0; y < kSize; ++y) {
        board_.append(QVector<int>(kSize, Empty));
    }
    gameOver_ = false;
    current_ = Black; // black first by default
}

void BoardWidget::startPVP()
{
    aiMode_ = false;
    aiSide_ = Black; // unused in PVP
    resetBoard();
    update();
}

void BoardWidget::startPVE()
{
    aiMode_ = true;
    aiSide_ = Black; // 固定 AI 执黑
    resetBoard();
    bool blackFirst = QRandomGenerator::global()->bounded(2) == 1;
    current_ = blackFirst ? Black : Red;
    update();
    if (current_ == aiSide_) {
        QTimer::singleShot(200, this, [this]{ maybeAIMove(); });
    }
}

void BoardWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // background
    p.fillRect(rect(), QColor(238, 198, 130));

    // compute cell size to fit
    int w = width() - margin_ * 2;
    int h = height() - margin_ * 2;
    cell_ = qMin(w, h) / (kSize - 1);
    int boardW = cell_ * (kSize - 1);
    int boardH = cell_ * (kSize - 1);
    int offsetX = (width() - boardW) / 2;
    int offsetY = (height() - boardH) / 2;

    // grid
    QPen gridPen(Qt::black);
    gridPen.setWidth(2);
    p.setPen(gridPen);
    for (int i = 0; i < kSize; ++i) {
        int x1 = offsetX;
        int x2 = offsetX + boardW;
        int y = offsetY + i * cell_;
        p.drawLine(x1, y, x2, y);
        int x = offsetX + i * cell_;
        int y1 = offsetY;
        int y2 = offsetY + boardH;
        p.drawLine(x, y1, x, y2);
    }

    // draw star points (for aesthetics)
    auto drawStar = [&](int gx, int gy){
        QPoint c = gridToPixel(QPoint(gx, gy));
        p.setBrush(Qt::black);
        p.drawEllipse(c, 4, 4);
    };
    if (kSize == 15) {
        int pts[3] = {3, 7, 11};
        for (int i: pts) for (int j: pts) drawStar(i, j);
    }

    // stones
    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x < kSize; ++x) {
            int v = board_[y][x];
            if (v == Empty) continue;
            QPoint c = gridToPixel(QPoint(x, y));
            QRadialGradient grad(c, cell_/2.0);
            if (v == Black) {
                grad.setColorAt(0, QColor(60,60,60));
                grad.setColorAt(1, QColor(10,10,10));
            } else {
                grad.setColorAt(0, QColor(255,120,120));
                grad.setColorAt(1, QColor(220,30,30));
            }
            p.setBrush(grad);
            p.setPen(Qt::NoPen);
            p.drawEllipse(c, cell_/2 - 2, cell_/2 - 2);
        }
    }
}

QPoint BoardWidget::gridToPixel(const QPoint& g) const
{
    int w = width() - margin_ * 2;
    int h = height() - margin_ * 2;
    int c = qMin(w, h) / (kSize - 1);
    int boardW = c * (kSize - 1);
    int boardH = c * (kSize - 1);
    int offsetX = (width() - boardW) / 2;
    int offsetY = (height() - boardH) / 2;
    return QPoint(offsetX + g.x() * c, offsetY + g.y() * c);
}

QPoint BoardWidget::posToGrid(const QPoint& pos, bool* ok) const
{
    int w = width() - margin_ * 2;
    int h = height() - margin_ * 2;
    int c = qMin(w, h) / (kSize - 1);
    int boardW = c * (kSize - 1);
    int boardH = c * (kSize - 1);
    int offsetX = (width() - boardW) / 2;
    int offsetY = (height() - boardH) / 2;

    // find nearest intersection
    int gx = qRound((pos.x() - offsetX) / double(c));
    int gy = qRound((pos.y() - offsetY) / double(c));
    QPoint g(gx, gy);
    bool inside = inBoard(g) &&
                  std::abs((pos.x() - (offsetX + gx * c))) <= c/2 &&
                  std::abs((pos.y() - (offsetY + gy * c))) <= c/2;
    if (ok) *ok = inside;
    return g;
}

bool BoardWidget::inBoard(const QPoint& g) const
{
    return g.x() >= 0 && g.x() < kSize && g.y() >= 0 && g.y() < kSize;
}

bool BoardWidget::placeAtGrid(const QPoint& g, int side)
{
    if (!inBoard(g)) return false;
    if (board_[g.y()][g.x()] != Empty) return false;
    board_[g.y()][g.x()] = side;
    return true;
}

void BoardWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (gameOver_) return;
    if (aiMode_ && current_ == aiSide_) return; // wait for AI

    bool ok = false;
    QPoint g = posToGrid(e->pos(), &ok);
    if (!ok) return;
    if (!placeAtGrid(g, current_)) return;

    update();
    if (checkWinFrom(g)) {
        gameOver_ = true;
        QString who = (current_ == Black) ? QStringLiteral("黑方") : QStringLiteral("红方");
        QMessageBox::information(this, QStringLiteral("游戏结束"), who + QStringLiteral("胜利！"));
        return;
    }

    current_ = (current_ == Black) ? Red : Black;

    if (aiMode_ && current_ == aiSide_) {
        QTimer::singleShot(150, this, [this]{ maybeAIMove(); });
    }
}

void BoardWidget::maybeAIMove()
{
    if (gameOver_) return;
    if (!aiMode_ || current_ != aiSide_) return;
    QPoint mv = aiBestMove();
    if (!inBoard(mv)) return;
    placeAtGrid(mv, current_);
    update();
    if (checkWinFrom(mv)) {
        gameOver_ = true;
        QString who = (current_ == Black) ? QStringLiteral("黑方(AI)") : QStringLiteral("红方(AI)");
        QMessageBox::information(this, QStringLiteral("游戏结束"), who + QStringLiteral("胜利！"));
        return;
    }
    current_ = (current_ == Black) ? Red : Black;
}

// Simple AI: 1) immediate win 2) block opponent win 3) max heuristic
QPoint BoardWidget::aiBestMove()
{
    // try all cells
    int bestScore = -1;
    QPoint best(-1, -1);
    int opp = (aiSide_ == Black) ? Red : Black;

    // 1) winning move
    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x < kSize; ++x) {
            if (board_[y][x] != Empty) continue;
            board_[y][x] = aiSide_;
            bool win = checkWinFrom(QPoint(x,y));
            board_[y][x] = Empty;
            if (win) return QPoint(x,y);
        }
    }
    // 2) block opponent winning move
    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x < kSize; ++x) {
            if (board_[y][x] != Empty) continue;
            board_[y][x] = opp;
            bool win = checkWinFrom(QPoint(x,y));
            board_[y][x] = Empty;
            if (win) return QPoint(x,y);
        }
    }
    // 3) heuristic score
    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x < kSize; ++x) {
            if (board_[y][x] != Empty) continue;
            QPoint g(x,y);
            int s = scorePoint(g, aiSide_);
            // add slight randomness among equals
            int tweak = QRandomGenerator::global()->bounded(3);
            s += tweak;
            if (s > bestScore) { bestScore = s; best = g; }
        }
    }
    if (best.x() < 0) {
        // fallback center
        return QPoint(kSize/2, kSize/2);
    }
    return best;
}

int BoardWidget::scorePoint(const QPoint& g, int side) const
{
    // sum over 4 directions
    static const int dirs[4][2] = {{1,0},{0,1},{1,1},{1,-1}};
    int total = 0;
    for (auto& d : dirs) total += lineScore(g, d[0], d[1], side);
    return total;
}

int BoardWidget::lineScore(const QPoint& g, int dx, int dy, int side) const
{
    // count contiguous on both sides if placed here
    auto countDir = [&](int sx, int sy){
        int cnt = 0;
        int x = g.x()+sx, y = g.y()+sy;
        while (x>=0 && x<kSize && y>=0 && y<kSize && board_[y][x]==side) { ++cnt; x+=sx; y+=sy; }
        return cnt;
    };
    int a = countDir(dx, dy);
    int b = countDir(-dx, -dy);
    int len = a + 1 + b;
    int score = 0;
    if (len >= 5) score += 100000;       // would win
    else if (len == 4) score += 1000;
    else if (len == 3) score += 150;
    else if (len == 2) score += 30;
    // also consider blocking opponent
    int opp = (side == Black) ? Red : Black;
    auto countOpp = [&](int sx, int sy){
        int cnt = 0; int x = g.x()+sx, y = g.y()+sy;
        while (x>=0 && x<kSize && y>=0 && y<kSize && board_[y][x]==opp) { ++cnt; x+=sx; y+=sy; }
        return cnt;
    };
    int oa = countOpp(dx, dy), ob = countOpp(-dx, -dy);
    int olen = oa + 1 + ob;
    if (olen >= 5) score += 50000;
    else if (olen == 4) score += 800;
    else if (olen == 3) score += 120;
    return score;
}

bool BoardWidget::checkWinFrom(const QPoint& g) const
{
    int side = board_[g.y()][g.x()];
    if (side == Empty) return false;
    static const int dirs[4][2] = {{1,0},{0,1},{1,1},{1,-1}};
    for (auto& d : dirs) {
        int dx = d[0], dy = d[1];
        int cnt = 1;
        // forward
        int x = g.x()+dx, y = g.y()+dy;
        while (x>=0 && x<kSize && y>=0 && y<kSize && board_[y][x]==side) { ++cnt; x+=dx; y+=dy; }
        // backward
        x = g.x()-dx; y = g.y()-dy;
        while (x>=0 && x<kSize && y>=0 && y<kSize && board_[y][x]==side) { ++cnt; x-=dx; y-=dy; }
        if (cnt >= 5) return true;
    }
    return false;
}

bool BoardWidget::checkWin(int side) const
{
    for (int y = 0; y < kSize; ++y)
        for (int x = 0; x < kSize; ++x)
            if (board_[y][x]==side && checkWinFrom(QPoint(x,y))) return true;
    return false;
}
