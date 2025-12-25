#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class GameBoard;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    GameBoard* board_ = nullptr;

    int rows_ = 9;
    int cols_ = 9;
    int mines_ = 10;

    void newGame();
    void chooseDifficultyAndStart();
    void buildMenus();
    void onGameOver(bool won);
    void onFlagsChanged(int flagsLeft);
};
#endif // MAINWINDOW_H
