#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gameboard.h"
#include "difficultydialog.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    buildMenus();
    statusBar()->showMessage(QStringLiteral("准备开始"));
    chooseDifficultyAndStart();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::buildMenus() {
    auto* gameMenu = menuBar()->addMenu(QStringLiteral("游戏"));
    auto* actNew = gameMenu->addAction(QStringLiteral("新游戏"));
    auto* actDiff = gameMenu->addAction(QStringLiteral("更改难度"));
    gameMenu->addSeparator();
    auto* actExit = gameMenu->addAction(QStringLiteral("退出"));

    connect(actNew, &QAction::triggered, this, [this]{ newGame(); });
    connect(actDiff, &QAction::triggered, this, [this]{ chooseDifficultyAndStart(); });
    connect(actExit, &QAction::triggered, this, [this]{ close(); });
}

void MainWindow::chooseDifficultyAndStart() {
    DifficultyDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        rows_ = dlg.rows();
        cols_ = dlg.cols();
        mines_ = dlg.mines();
        newGame();
    }
}

void MainWindow::newGame() {
    if (board_) {
        board_->deleteLater();
        board_ = nullptr;
    }
    board_ = new GameBoard(rows_, cols_, mines_, this);
    setCentralWidget(board_);
    connect(board_, &GameBoard::gameOver, this, &MainWindow::onGameOver);
    connect(board_, &GameBoard::flagsChanged, this, &MainWindow::onFlagsChanged);
    onFlagsChanged(mines_);
}

void MainWindow::onFlagsChanged(int flagsLeft) {
    statusBar()->showMessage(QStringLiteral("剩余旗帜：%1").arg(flagsLeft));
}

void MainWindow::onGameOver(bool won) {
    QMessageBox::information(this, won ? QStringLiteral("胜利") : QStringLiteral("失败"),
                             won ? QStringLiteral("恭喜，你赢了！") : QStringLiteral("踩雷了，游戏结束！"));
}

