#include "gamewindow.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include "sudokudelegate.h"

GameWindow::GameWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle("数独对局");
    // 固定对局窗口大小
    setFixedSize(540, 640);
    auto* layout = new QVBoxLayout(this);
    table_ = new QTableWidget(9,9,this);
    table_->horizontalHeader()->setVisible(false);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
    // 使用自定义委托绘制粗线条九宫格边界
    table_->setShowGrid(false);
    table_->setItemDelegate(new SudokuDelegate(table_));
    layout->addWidget(table_);
    saveBtn_ = new QPushButton("保存一步", this);
    status_ = new QLabel("", this);
    layout->addWidget(saveBtn_);
    layout->addWidget(status_);
    setupBoard();
    connect(table_, &QTableWidget::cellChanged, this, &GameWindow::onCellChanged);
    connect(saveBtn_, &QPushButton::clicked, this, &GameWindow::onSaveStep);
}

void GameWindow::setupBoard() {
    for (int r=0;r<9;r++) for (int c=0;c<9;c++) {
        auto* item = new QTableWidgetItem("");
        item->setTextAlignment(Qt::AlignCenter);
        table_->setItem(r,c,item);
    }
}

void GameWindow::refreshBoard() {
    // 避免刷新时触发 cellChanged 导致误判或保存
    table_->blockSignals(true);
    for (int r=0;r<9;r++) for (int c=0;c<9;c++) {
        auto* item = table_->item(r,c);
        int v = current_[r][c];
        item->setText(v==0?"":QString::number(v));
        bool fixed = puzzle_[r][c]!=0;
        item->setFlags(fixed ? (item->flags() & ~Qt::ItemIsEditable) : (item->flags() | Qt::ItemIsEditable));
        item->setBackground(fixed ? QColor(230,230,230) : QColor(255,255,255));
    }
    table_->blockSignals(false);
}

void GameWindow::startNew(const SudokuLogic::Grid& solved, const SudokuLogic::Grid& puzzle, const QString& saveDir){
    solved_ = solved; puzzle_ = puzzle; current_ = puzzle; saveDir_ = saveDir;
    QDir().mkpath(saveDir_);
    // 保存完整版 *_ply.txt
    QFile f(saveDir_+"/grid_ply.txt");
    if (f.open(QIODevice::WriteOnly|QIODevice::Text)) {
        f.write(SudokuLogic::toText(solved_).toUtf8());
        f.close();
    }
    // 保存初始 puzzle
    QFile p(saveDir_+"/grid_init.txt");
    if (p.open(QIODevice::WriteOnly|QIODevice::Text)) { p.write(SudokuLogic::toText(puzzle_).toUtf8()); p.close(); }
    refreshBoard();
    saveGridStep();
}

bool GameWindow::loadFromDir(const QString& saveDir) {
    saveDir_ = saveDir;
    QFile s(saveDir_+"/grid_ply.txt");
    QFile p(saveDir_+"/grid_init.txt");
    QFile c(saveDir_+"/grid_current.txt");
    if (!s.exists() || !p.exists()) return false;
    if (s.open(QIODevice::ReadOnly|QIODevice::Text)) { solved_ = SudokuLogic::fromText(QString::fromUtf8(s.readAll())); s.close(); }
    if (p.open(QIODevice::ReadOnly|QIODevice::Text)) { puzzle_ = SudokuLogic::fromText(QString::fromUtf8(p.readAll())); p.close(); }
    if (c.exists() && c.open(QIODevice::ReadOnly|QIODevice::Text)) { current_ = SudokuLogic::fromText(QString::fromUtf8(c.readAll())); c.close(); }
    else current_ = puzzle_;
    refreshBoard();
    return true;
}

void GameWindow::onCellChanged(int r, int c) {
    auto* item = table_->item(r,c);
    if (!item) return;
    QString t = item->text().trimmed();
    if (t.isEmpty()) { current_[r][c]=0; return; }
    bool ok=false; int v=t.toInt(&ok);
    if (!ok || v<1 || v>9) { item->setText(""); return; }
    if (puzzle_[r][c]!=0) { item->setText(QString::number(puzzle_[r][c])); return; }
    if (SudokuLogic::isValidMove(current_, r, c, v)) {
        current_[r][c]=v;
        status_->setText("");
        saveGridStep();
        if (SudokuLogic::isComplete(current_)) {
            status_->setText("已完成！");
            emit gameFinished();
        }
    } else {
        item->setText("");
        status_->setText("非法输入，行列或宫冲突");
    }
}

void GameWindow::onSaveStep() { saveGridStep(); }

void GameWindow::saveGridStep() {
    if (saveDir_.isEmpty()) return;
    QFile cur(saveDir_+"/grid_current.txt");
    if (cur.open(QIODevice::WriteOnly|QIODevice::Text)) { cur.write(SudokuLogic::toText(current_).toUtf8()); cur.close(); }
    // 追加操作日志
    QFile log(saveDir_+"/steps.log");
    if (log.open(QIODevice::Append|QIODevice::Text)) {
        QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        log.write((ts+"\n").toUtf8());
        log.close();
    }
}
