#include "gameboard.h"
#include "cellbutton.h"

#include <QRandomGenerator>
#include <QMessageBox>
#include <QQueue>
#include <QLabel>

GameBoard::GameBoard(int rows, int cols, int mines, QWidget* parent)
    : QWidget(parent), rows_(rows), cols_(cols), mines_(mines) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    grid_ = new QGridLayout(this);
    grid_->setSpacing(1);
    grid_->setContentsMargins(4, 4, 4, 4);
    reset(rows, cols, mines);
}

void GameBoard::reset(int rows, int cols, int mines) {
    rows_ = rows; cols_ = cols; mines_ = mines;
    flagsLeft_ = mines_;
    firstClick_ = true;
    clearUI();
    initBoard();
    buildUI();
    emit flagsChanged(flagsLeft_);
}

void GameBoard::clearUI() {
    if (!buttons_.isEmpty()) {
        for (int r = 0; r < buttons_.size(); ++r) {
            for (int c = 0; c < buttons_[r].size(); ++c) {
                if (buttons_[r][c]) {
                    grid_->removeWidget(buttons_[r][c]);
                    delete buttons_[r][c];
                }
            }
        }
    }
    buttons_.clear();
}

void GameBoard::initBoard() {
    cells_ = QVector<QVector<Cell>>(rows_, QVector<Cell>(cols_));
}

void GameBoard::buildUI() {
    buttons_.resize(rows_);
    for (int r = 0; r < rows_; ++r) {
        buttons_[r].resize(cols_);
        for (int c = 0; c < cols_; ++c) {
            auto* btn = new CellButton(r, c, this);
            buttons_[r][c] = btn;
            grid_->addWidget(btn, r, c);
            connect(btn, &CellButton::leftClicked, this, &GameBoard::onLeftClicked);
            connect(btn, &CellButton::rightClicked, this, &GameBoard::onRightClicked);
        }
    }
}

void GameBoard::placeMines(int firstR, int firstC) {
    // Place mines randomly; ensure first click isn't a mine and its neighbors safe.
    int placed = 0;
    while (placed < mines_) {
        int r = QRandomGenerator::global()->bounded(rows_);
        int c = QRandomGenerator::global()->bounded(cols_);
        if (cells_[r][c].isMine) continue;
        if (firstR >= 0) {
            // avoid first cell and its neighbors
            if (qAbs(r - firstR) <= 1 && qAbs(c - firstC) <= 1) continue;
        }
        cells_[r][c].isMine = true;
        ++placed;
    }
}

void GameBoard::computeAdjacency() {
    static const int dr[8] = {-1,-1,-1,0,0,1,1,1};
    static const int dc[8] = {-1,0,1,-1,1,-1,0,1};
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            if (cells_[r][c].isMine) { cells_[r][c].adj = -1; continue; }
            int cnt = 0;
            for (int k = 0; k < 8; ++k) {
                int nr = r + dr[k], nc = c + dc[k];
                if (inBounds(nr, nc) && cells_[nr][nc].isMine) ++cnt;
            }
            cells_[r][c].adj = cnt;
        }
    }
}

void GameBoard::onLeftClicked(int r, int c) {
    if (cells_[r][c].flagged || cells_[r][c].revealed) return;
    if (firstClick_) {
        placeMines(r, c);
        computeAdjacency();
        firstClick_ = false;
    }
    if (cells_[r][c].isMine) {
        revealAllMines();
        emit gameOver(false);
        return;
    }
    revealCell(r, c);
    if (checkWin()) {
        emit gameOver(true);
    }
}

void GameBoard::onRightClicked(int r, int c) {
    if (cells_[r][c].revealed) return;
    cells_[r][c].flagged = !cells_[r][c].flagged;
    flagsLeft_ += cells_[r][c].flagged ? -1 : 1;
    buttons_[r][c]->setFlagged(cells_[r][c].flagged);
    emit flagsChanged(flagsLeft_);
}

void GameBoard::revealCell(int r, int c) {
    if (!inBounds(r, c)) return;
    Cell &cell = cells_[r][c];
    if (cell.revealed || cell.flagged) return;
    cell.revealed = true;
    buttons_[r][c]->setRevealed(true);
    updateButtonAppearance(r, c);
    if (cell.adj == 0) {
        floodReveal(r, c);
    }
}

void GameBoard::floodReveal(int r, int c) {
    static const int dr[8] = {-1,-1,-1,0,0,1,1,1};
    static const int dc[8] = {-1,0,1,-1,1,-1,0,1};
    QQueue<QPair<int,int>> q;
    q.enqueue({r,c});
    while (!q.isEmpty()) {
        QPair<int,int> pr = q.dequeue();
        int cr = pr.first;
        int cc = pr.second;
        for (int k = 0; k < 8; ++k) {
            int nr = cr + dr[k], nc = cc + dc[k];
            if (!inBounds(nr, nc)) continue;
            Cell &ncell = cells_[nr][nc];
            if (ncell.revealed || ncell.flagged || ncell.isMine) continue;
            ncell.revealed = true;
            buttons_[nr][nc]->setRevealed(true);
            updateButtonAppearance(nr, nc);
            if (ncell.adj == 0) q.enqueue(QPair<int,int>(nr, nc));
        }
    }
}

void GameBoard::revealAllMines() {
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            if (cells_[r][c].isMine) {
                buttons_[r][c]->setText("*");
                buttons_[r][c]->setStyleSheet("color: red; background: #fdd;");
            }
            buttons_[r][c]->setEnabled(false);
        }
    }
}

void GameBoard::updateButtonAppearance(int r, int c) {
    const Cell &cell = cells_[r][c];
    if (!cell.revealed) return;
    if (cell.isMine) return;
    if (cell.adj > 0) {
        buttons_[r][c]->setText(QString::number(cell.adj));
        static const char* colors[] = {"#0000ff","#008200","#ff0000","#000084","#840000","#008284","#840084","#000000"};
        int idx = qBound(1, cell.adj, 8) - 1;
        buttons_[r][c]->setStyleSheet(QString("color: %1; background: #ddd;").arg(colors[idx]));
    } else {
        buttons_[r][c]->setText("");
        buttons_[r][c]->setStyleSheet("background: #ddd;");
    }
}

bool GameBoard::checkWin() const {
    int safe = rows_ * cols_ - mines_;
    int revealed = 0;
    for (int r = 0; r < rows_; ++r)
        for (int c = 0; c < cols_; ++c)
            if (cells_[r][c].revealed && !cells_[r][c].isMine) ++revealed;
    return revealed == safe;
}
