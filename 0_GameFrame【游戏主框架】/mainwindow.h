#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class GameWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSelectGame();
    void onSettings();
    void onExit();

private:
    Ui::MainWindow *ui;
    GameWindow *m_gameWindow;
};
#endif // MAINWINDOW_H
