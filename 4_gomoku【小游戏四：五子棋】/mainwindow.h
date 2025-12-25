#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class BoardWidget;

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
    BoardWidget* board_ = nullptr;
    void setupUiAndLayout();
private slots:
    void onStartPVP();
    void onStartPVE();
    void onExit();
};
#endif // MAINWINDOW_H
