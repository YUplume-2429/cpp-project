//这个文件是完整的头文件

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFont>
#include <QSlider>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include "snake.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    QLabel *label;
    QPushButton *startBtn;
    QPushButton *setBtn;
    QPushButton *rankBtn;
    QPushButton *autoBtn;  // 自动演示按钮
    QPushButton *quitBtn;
    snake* wni;
    snake* autowni;   // 自动演示窗口

public slots:
    void startBtnclick();  //槽函数
    void quitBtnclick();
    void setBtnclick();
    void rankBtnclick();
    void autoBtnclick();  // 自动演示槽函数

    //新添加的槽函数
    void onEasySelected();
    void onNormalSelected();
    void onHardSelected();
    
    // 设置窗口相关槽函数
    void onSettingsConfirm();
    void onSettingsCancel();
    void onMapSelectionChanged();
    void onSkinSelectionChanged();

signals:


private:
    Ui::Widget *ui;

    QWidget* difficultyWindow;  // 难度选择窗口
    QWidget* scoreRankWindow;   // 分数排名窗口
    QWidget* settingsWindow;    // 设置窗口
    QPushButton* easyBtn;
    QPushButton* normalBtn;
    QPushButton* hardBtn;
    
    // 设置窗口控件
    QButtonGroup* mapButtonGroup;
    QButtonGroup* skinButtonGroup;
    QRadioButton* map1Radio;
    QRadioButton* map2Radio;
    QRadioButton* map3Radio;
    QRadioButton* skin1Radio;
    QRadioButton* skin2Radio;
    QRadioButton* skin3Radio;
    QLabel* mapPreviewLabel;
    QLabel* skinPreviewLabel;
    
    // 设置数据
    int currentMapIndex;
    int currentSkinIndex;

    // 初始化难度选择窗口
    void initDifficultyWindow();
    void initScoreRankWindow();
    void initSettingsWindow();
    void updateMapPreview(int mapIndex);
    void updateSkinPreview(int skinIndex);
};
#endif // WIDGET_H
