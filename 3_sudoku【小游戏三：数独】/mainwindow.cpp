#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDateTime>
#include "recordsdialog.h"
#include "difficultydialog.h"
#include "gamewindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("数独游戏");
    // 固定主界面大小
    setFixedSize(800, 600);
    // 设置背景图（路径预留，可替换为实际文件）
    // 提示：将下面的占位路径替换为实际图片路径或资源路径
    this->setStyleSheet("QMainWindow { background-image: url(':/images/images/bg.png'); background-repeat: no-repeat; background-position: center; }");

    connect(ui->continueButton, &QPushButton::clicked, this, &MainWindow::onContinueGame);
    connect(ui->newGameButton, &QPushButton::clicked, this, &MainWindow::onNewGame);
    connect(ui->clearRecordsButton, &QPushButton::clicked, this, &MainWindow::onClearRecords);
    connect(ui->exitButton, &QPushButton::clicked, this, &MainWindow::onExitGame);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Placeholder implementations; actual logic will be added with corresponding dialogs and storage
void MainWindow::onContinueGame()
{
    // 列出最近10项记录（按时间排序的目录）
    QString base = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("records");
    QDir dir(base);
    if (!dir.exists()) dir.mkpath(".");
    auto entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    QList<GameRecord> recs;
    for (int i=0;i<entries.size() && i<10;i++) {
        QString p = dir.absoluteFilePath(entries[i]);
        bool ok = QFile::exists(p+"/grid_ply.txt");
        QString title = entries[i];
        recs.append({title, p, ok});
    }
    RecordsDialog dlg(this);
    dlg.setRecords(recs);
    connect(&dlg, &RecordsDialog::recordChosen, this, [this](const GameRecord& rec){
        // 显示选择的记录对应的目录/文件名
        QMessageBox::information(this, "选择的记录", QString("目录：%1\n文件：grid_current.txt").arg(rec.path));
        if (!rec.exists) { QMessageBox::information(this, "提示", "没有可继续的游玩记录"); return; }
        GameWindow* gw = new GameWindow(this);
        if (gw->loadFromDir(rec.path)) gw->show(); else { QMessageBox::warning(this, "错误", "记录读取失败"); gw->deleteLater(); }
    });
    dlg.exec();
}

void MainWindow::onNewGame()
{
    DifficultyDialog dd(this);
    Difficulty chosen = Difficulty::Normal;
    connect(&dd, &DifficultyDialog::difficultyChosen, this, [&](Difficulty d){ chosen = d; });
    if (dd.exec() != QDialog::Accepted) return;
    // 细化难度：以线索数控制，并保证唯一解
    int clues = 36; // 默认普通
    bool symmetric = false; // 不使用对称约束
    if (chosen==Difficulty::Easy) { clues = 44; }
    else if (chosen==Difficulty::Normal) { clues = 36; }
    else if (chosen==Difficulty::Hard) { clues = 28; }

    auto solved = SudokuLogic::generateSolved();
    auto puzzle = SudokuLogic::makeUniquePuzzle(solved, clues, symmetric);

    QString base = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("records");
    QDir().mkpath(base);
    QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString dirName = QString("%1_%2").arg(stamp).arg(chosen==Difficulty::Easy?"easy":(chosen==Difficulty::Normal?"normal":"hard"));
    QString saveDir = QDir(base).absoluteFilePath(dirName);

    // 维护最近10项（目录排序由时间决定，无需额外索引；超出不删除，只是不显示）
    GameWindow* gw = new GameWindow(this);
    gw->startNew(solved, puzzle, saveDir);
    gw->show();
}

void MainWindow::onClearRecords()
{
    QString base = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("records");
    QDir dir(base);
    if (!dir.exists()) { QMessageBox::information(this, "提示", "目前没有记录可清空"); return; }
    auto files = dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
    for (const auto& name : files) {
        QFileInfo fi(dir.absoluteFilePath(name));
        if (fi.isDir()) QDir(fi.absoluteFilePath()).removeRecursively();
        else QFile::remove(fi.absoluteFilePath());
    }
    QMessageBox::information(this, "提示", "记录已清空");
}

void MainWindow::onExitGame()
{
    close();
}

