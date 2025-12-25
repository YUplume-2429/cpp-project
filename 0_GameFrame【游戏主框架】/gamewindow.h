#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QDialog>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class GameWindow; }
QT_END_NAMESPACE

class GameWindow : public QDialog
{
    Q_OBJECT
public:
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow();

    // 预留接口：由用户填充每个游戏的图标与名称与可执行路径
    void setGameInfo(int index, const QString &name, const QString &iconPath, const QString &exePath);

private slots:
    void onGameClicked(int index);

private:
    Ui::GameWindow *ui;
    struct GameItem {
        QString name;
        QString iconPath;
        QString exePath;
    };
    GameItem m_items[6];
};

#endif // GAMEWINDOW_H
