#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPoint>

class BoardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoardWidget(QWidget *parent = nullptr);
    enum Cell { Empty = 0, Black = 1, Red = 2 };

    void startPVP();
    void startPVE();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    static const int kSize = 15; // 15x15
    QVector<QVector<int>> board_;
    bool gameOver_ = false;
    bool aiMode_ = false;
    int current_ = Black; // current player
    int aiSide_ = Black;  // fixed AI side in PVE

    // metrics
    int margin_ = 30;
    int cell_ = 36; // pixel size per cell

    bool placeAtGrid(const QPoint& pos, int side);
    QPoint posToGrid(const QPoint& pos, bool* ok) const;
    QPoint gridToPixel(const QPoint& g) const;
    bool inBoard(const QPoint& g) const;
    bool checkWinFrom(const QPoint& g) const;
    bool checkWin(int side) const;
    void resetBoard();

    // AI
    void maybeAIMove();
    QPoint aiBestMove();
    int scorePoint(const QPoint& g, int side) const;
    int lineScore(const QPoint& g, int dx, int dy, int side) const;
};

#endif // BOARDWIDGET_H
