#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gamewindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_gameWindow(nullptr)
{
    ui->setupUi(this);

    // Central widget and vertical layout
    QWidget *central = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout(central);

    QPushButton *btnSelect = new QPushButton(tr("选择游戏"), central);
    QPushButton *btnSettings = new QPushButton(tr("设置"), central);
    QPushButton *btnExit = new QPushButton(tr("退出"), central);

    btnSelect->setMinimumHeight(40);
    btnSettings->setMinimumHeight(40);
    btnExit->setMinimumHeight(40);

    vbox->addWidget(btnSelect);
    vbox->addWidget(btnSettings);
    vbox->addWidget(btnExit);
    vbox->addStretch();
    setCentralWidget(central);

    connect(btnSelect, &QPushButton::clicked, this, &MainWindow::onSelectGame);
    connect(btnSettings, &QPushButton::clicked, this, &MainWindow::onSettings);
    connect(btnExit, &QPushButton::clicked, this, &MainWindow::onExit);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSelectGame()
{
    if (!m_gameWindow) {
        m_gameWindow = new GameWindow(this);
        // 假设名称：exe位于与 GameFrame.exe 同级目录
        m_gameWindow->setGameInfo(0, tr("贪吃蛇"), ":/images/images/snake.png", "SnakeGame.exe");
        m_gameWindow->setGameInfo(1, tr("坦克大战"), ":/images/images/tankbattle.png", "tankbattle.exe");
        m_gameWindow->setGameInfo(2, tr("数独"), ":/images/images/sudoku.png", "sudoku.exe");
        m_gameWindow->setGameInfo(3, tr("五子棋对战"), ":/images/images/gomoku.png", "gomoku.exe");
        m_gameWindow->setGameInfo(4, tr("扫雷"), ":/images/images/minesweeper.png", "minesweeper.exe");
        m_gameWindow->setGameInfo(5, tr("ZombieMob"), ":/images/images/ZombieMob.png", "ZombieMobGame.exe");
    }
    m_gameWindow->show();
    m_gameWindow->raise();
    m_gameWindow->activateWindow();
}

void MainWindow::onSettings()
{
    // 预留：设置窗口或对话框，当前功能待定
}

void MainWindow::onExit()
{
    close();
}

