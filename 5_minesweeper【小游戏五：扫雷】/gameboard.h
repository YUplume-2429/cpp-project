#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <QWidget>
#include <QGridLayout>
#include <QVector>

class CellButton;

class GameBoard : public QWidget {
    Q_OBJECT
public:
    explicit GameBoard(int rows, int cols, int mines, QWidget* parent = nullptr);

    int rows() const { return rows_; }
    int cols() const { return cols_; }
    int mines() const { return mines_; }

signals:
    void gameOver(bool won);
    void flagsChanged(int flagsLeft);

public slots:
    void reset(int rows, int cols, int mines);

private slots:
    void onLeftClicked(int r, int c);
    void onRightClicked(int r, int c);

private:
    struct Cell {
        bool isMine = false;
        int adj = 0;
        bool revealed = false;
        bool flagged = false;
    };

    void buildUI();
    void clearUI();
    void initBoard();
    void placeMines(int firstR = -1, int firstC = -1);
    void computeAdjacency();
    void revealCell(int r, int c);
    void floodReveal(int r, int c);
    void revealAllMines();
    bool inBounds(int r, int c) const { return r >= 0 && r < rows_ && c >= 0 && c < cols_; }
    void updateButtonAppearance(int r, int c);
    bool checkWin() const;

    int rows_;
    int cols_;
    int mines_;
    int flagsLeft_ = 0;
    bool firstClick_ = true;

    QVector<QVector<Cell>> cells_;
    QVector<QVector<CellButton*>> buttons_;
    QGridLayout* grid_ = nullptr;
};

#endif // GAMEBOARD_H
