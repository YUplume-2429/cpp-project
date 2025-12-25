#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include "sudokulogic.h"

class GameWindow : public QDialog {
    Q_OBJECT
public:
    explicit GameWindow(QWidget* parent=nullptr);
    void startNew(const SudokuLogic::Grid& solved, const SudokuLogic::Grid& puzzle, const QString& saveDir);
    bool loadFromDir(const QString& saveDir); // 返回是否成功

signals:
    void gameFinished();

private slots:
    void onCellChanged(int r, int c);
    void onSaveStep();

private:
    void setupBoard();
    void refreshBoard();
    void saveGridStep();

    QTableWidget* table_;
    QPushButton* saveBtn_;
    QLabel* status_;
    SudokuLogic::Grid solved_{};
    SudokuLogic::Grid puzzle_{};
    SudokuLogic::Grid current_{};
    QString saveDir_;
};

#endif // GAMEWINDOW_H