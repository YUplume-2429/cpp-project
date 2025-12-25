#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QRandomGenerator>
#include <QStatusBar>
#include "boardwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUiAndLayout();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUiAndLayout()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    auto* root = new QVBoxLayout(central);
    auto* btnRow = new QHBoxLayout();

    auto* btnPVP = new QPushButton(QStringLiteral("双人对战"), central);
    auto* btnPVE = new QPushButton(QStringLiteral("人机对战"), central);
    auto* btnExit = new QPushButton(QStringLiteral("退出游戏"), central);

    btnRow->addWidget(btnPVP);
    btnRow->addWidget(btnPVE);
    btnRow->addWidget(btnExit);
    btnRow->addStretch();

    board_ = new BoardWidget(central);

    root->addLayout(btnRow);
    root->addWidget(board_, 1);

    connect(btnPVP, &QPushButton::clicked, this, &MainWindow::onStartPVP);
    connect(btnPVE, &QPushButton::clicked, this, &MainWindow::onStartPVE);
    connect(btnExit, &QPushButton::clicked, this, &MainWindow::onExit);

    statusBar()->showMessage(QStringLiteral("请选择模式开始游戏"));
}

void MainWindow::onStartPVP()
{
    if (board_) board_->startPVP();
    statusBar()->showMessage(QStringLiteral("双人对战：黑先"));
}

void MainWindow::onStartPVE()
{
    if (board_) board_->startPVE();
    statusBar()->showMessage(QStringLiteral("人机对战：先后手随机，AI固定执黑"));
}

void MainWindow::onExit()
{
    close();
}

