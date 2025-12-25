#include "widget.h"
#include "ui_widget.h"
#include "snake.h"
#include <QPalette>
#include <QIcon>
#include <QBrush>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFont>
#include <QDebug>
#include <QSlider>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
/*
    // 创建标签控件
       QLabel *imageLabel = new QLabel(this);

       // 加载图片
       QPixmap pixmap(":/images/images/Title.png");

       // 设置图片到标签
       imageLabel->setPixmap(pixmap);

       // 可选：保持图片比例缩放
       imageLabel->setPixmap(pixmap.scaled(400, 400, Qt::KeepAspectRatio));

       // 可选：居中显示
       imageLabel->setAlignment(Qt::AlignCenter);

       // 设置标签位置和大小
       imageLabel->setGeometry(50, 50, pixmap.width(), pixmap.height());
*/
    this->setWindowTitle("贪吃蛇");
    this->setWindowIcon(QIcon(":/images/images/SnakeHead1.png")); // 如果有图标资源
    this->setGeometry(QRect(600,200,600,600));
    this->setFixedSize(600, 600);

    QPalette palette;
    palette.setColor(QPalette::Window,Qt::white);
    QPixmap bgPixmap(":/images/images/cover1.png");
    QPixmap scaledPixmap = bgPixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    palette.setBrush(QPalette::Window, QBrush(scaledPixmap));
    this->setPalette(palette);

    // 标签初始化
    this->label = new QLabel(this);

    // 初始化snake窗口
    wni = new snake(this);
    wni->hide(); // 初始隐藏
    
    // 初始化自动演示窗口
    autowni = new snake(this);
    autowni->hide(); // 初始隐藏

    // 创建难度选择窗口
    initDifficultyWindow();

    // 初始化设置数据
    currentMapIndex = 0;     // 默认地图1
    currentSkinIndex = 0;    // 默认皮肤1
    
    // 创建分数排名窗口
    initScoreRankWindow();
    
    // 创建设置窗口
    initSettingsWindow();

    this->startBtn = new QPushButton(this);
    QLabel* startLl = new QLabel();
    startLl->setText("开始游戏");
    startLl->setParent(startBtn);
    startLl->setAlignment(Qt::AlignCenter);
    QFont ft;
    ft.setPointSize(8);
    ft.setFamily("Arial");
    ft.setBold(true);
    startLl->setFont(ft);
    startBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(0, 0, 0, 0);" // 完全透明背景
        "   color: black;"                       // 设置文字颜色
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(200, 200, 200, 100);" // 悬停时半透明
        "}"
    );
    QVBoxLayout* startLlhBox = new QVBoxLayout(startBtn);
    startLlhBox->addWidget(startLl);
    startBtn->move(220,200);
    startBtn->resize(150,60);


    // 添加设置按钮
    this->setBtn = new QPushButton(this);
    QLabel* setLl = new QLabel();
    setLl->setText("游戏设置");
    setLl->setParent(setBtn);
    setLl->setAlignment(Qt::AlignCenter);
    QFont ft1;
    ft1.setPointSize(8);
    ft1.setFamily("Arial");
    ft1.setBold(true);
    setLl->setFont(ft1);
    setBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(0, 0, 0, 0);" // 完全透明背景
        "   color: rgba(227, 66, 68, 255);"                       // 设置文字颜色
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(200, 200, 200, 100);" // 悬停时半透明
        "}"
    );
    QVBoxLayout* setLlhBox = new QVBoxLayout(setBtn);
    setLlhBox->addWidget(setLl);
    setBtn->move(220,280);
    setBtn->resize(150,60);

    // 添加分数排名按钮
    this->rankBtn = new QPushButton(this);
    QLabel* rankLl = new QLabel();
    rankLl->setText("分数排名");
    rankLl->setParent(rankBtn);
    rankLl->setAlignment(Qt::AlignCenter);
    QFont ft2;
    ft2.setPointSize(8);
    ft2.setFamily("Arial");
    ft2.setBold(true);
    rankLl->setFont(ft2);
    rankBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(0, 0, 0, 0);" // 完全透明背景
        "   color: black;"                       // 设置文字颜色
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(200, 200, 200, 100);" // 悬停时半透明
        "}"
    );
    QVBoxLayout* helpLlhBox = new QVBoxLayout(rankBtn);
    helpLlhBox->addWidget(rankLl);
    rankBtn->move(220,360);
    rankBtn->resize(150,60);

    // 添加自动演示按钮
    this->autoBtn = new QPushButton(this);
    QLabel* autoLl = new QLabel();
    autoLl->setText("自动演示");
    autoLl->setParent(autoBtn);
    autoLl->setAlignment(Qt::AlignCenter);
    QFont ft4;
    ft4.setPointSize(8);
    ft4.setFamily("Arial");
    ft4.setBold(true);
    autoLl->setFont(ft4);
    autoBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(0, 0, 0, 0);" // 完全透明背景
        "   color: black;"                       // 设置文字颜色
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(200, 200, 200, 100);" // 悬停时半透明
        "}"
    );
    QVBoxLayout* autoLlhBox = new QVBoxLayout(autoBtn);
    autoLlhBox->addWidget(autoLl);
    autoBtn->move(220,440);
    autoBtn->resize(150,60);

    // 添加退出按钮
    this->quitBtn = new QPushButton(this);
    QLabel* quitLl = new QLabel();
    quitLl->setText("退出游戏");
    quitLl->setParent(quitBtn); // 修复：应该是quitBtn而不是startBtn
    quitLl->setAlignment(Qt::AlignCenter);
    QFont ft3;
    ft3.setPointSize(8);
    ft3.setFamily("Arial");
    ft3.setBold(true);
    quitLl->setFont(ft3);
    quitBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(0, 0, 0, 0);" // 完全透明背景
        "   color: black;"                       // 设置文字颜色
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(200, 200, 200, 100);" // 悬停时半透明
        "}"
    );
    QVBoxLayout* quitLlhBox = new QVBoxLayout(quitBtn);
    quitLlhBox->addWidget(quitLl);
    quitBtn->move(220,520);
    quitBtn->resize(150,60);

    // 信号与槽的关联
    this->connect(startBtn, SIGNAL(clicked()), this, SLOT(startBtnclick()));
    this->connect(quitBtn, SIGNAL(clicked()), this, SLOT(quitBtnclick()));
    this->connect(setBtn, SIGNAL(clicked()), this, SLOT(setBtnclick()));
    this->connect(rankBtn, SIGNAL(clicked()), this, SLOT(rankBtnclick()));
    this->connect(autoBtn, SIGNAL(clicked()), this, SLOT(autoBtnclick()));
}

//void Widget::startBtnclick() {
//    this->hide();
//    wni->show();
//}


// 开始按钮点击槽函数
void Widget::startBtnclick()
{
    // 隐藏当前窗口
    this->hide();

    // 居中显示难度选择窗口
    QPoint centerPos = this->geometry().center() - difficultyWindow->rect().center();
    difficultyWindow->move(centerPos);
    difficultyWindow->show();
}


void Widget::initDifficultyWindow()
{
    // 创建难度选择窗口
    difficultyWindow = new QWidget(this);
    difficultyWindow->setWindowTitle("选择难度");
    difficultyWindow->setFixedSize(300, 200);
    difficultyWindow->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    difficultyWindow->hide(); // 初始隐藏

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(difficultyWindow);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 添加标题
    QLabel* titleLabel = new QLabel("请选择游戏难度：", difficultyWindow);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont("Arial", 14, QFont::Bold);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // 添加间距
    mainLayout->addSpacing(20);

    // 创建难度按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setAlignment(Qt::AlignCenter);

    // 创建难度按钮
    easyBtn = new QPushButton("简单", difficultyWindow);
    normalBtn = new QPushButton("普通", difficultyWindow);
    hardBtn = new QPushButton("困难", difficultyWindow);

    // 设置按钮样式
    QFont btnFont("Arial", 12);
    easyBtn->setFont(btnFont);
    normalBtn->setFont(btnFont);
    hardBtn->setFont(btnFont);

    easyBtn->setFixedSize(80, 40);
    normalBtn->setFixedSize(80, 40);
    hardBtn->setFixedSize(80, 40);

    // 添加按钮到布局
    buttonLayout->addWidget(easyBtn);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(normalBtn);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(hardBtn);

    // 将按钮布局添加到主布局
    mainLayout->addLayout(buttonLayout);

    // 连接难度按钮的信号到槽函数
    connect(easyBtn, &QPushButton::clicked, this, &Widget::onEasySelected);
    connect(normalBtn, &QPushButton::clicked, this, &Widget::onNormalSelected);
    connect(hardBtn, &QPushButton::clicked, this, &Widget::onHardSelected);
}

void Widget::initScoreRankWindow()
{
    // 创建分数排名窗口
    scoreRankWindow = new QWidget(this);
    scoreRankWindow->setWindowTitle("分数排名");
    scoreRankWindow->setFixedSize(600, 850);
    scoreRankWindow->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    scoreRankWindow->hide(); // 初始隐藏

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(scoreRankWindow);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(8); // 设置子控件之间的间距

    // 添加标题
    QLabel* titleLabel = new QLabel("🏆 分数排行榜 🏆", scoreRankWindow);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont("Arial", 14, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setFixedHeight(80); // 设置固定高度
    titleLabel->setStyleSheet("color: #233d56; margin-bottom: 15px; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    // 添加间距
    mainLayout->addSpacing(10);

    // 添加分数列表标签
    for(int i = 0; i < 10; ++i) {
        QLabel* scoreLabel = new QLabel(scoreRankWindow);
        scoreLabel->setAlignment(Qt::AlignLeft);
        QFont scoreFont("Arial", 14);
        scoreLabel->setFont(scoreFont);
        scoreLabel->setMinimumHeight(45); // 设置最小高度增加行间距
        scoreLabel->setStyleSheet(
            "QLabel {"
            "   padding: 10px;"
            "   margin: 3px;"
            "   border-radius: 8px;"
            "   background-color: #ecf0f1;"
            "}"
        );
        scoreLabel->setObjectName(QString("scoreLabel_%1").arg(i)); // 设置对象名称以便后续查找
        mainLayout->addWidget(scoreLabel);
    }

    // 添加关闭按钮
    QPushButton* closeBtn = new QPushButton("关闭", scoreRankWindow);
    closeBtn->setFixedSize(120, 40);
    QFont btnFont("Arial", 14);
    closeBtn->setFont(btnFont);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
    );
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setAlignment(Qt::AlignCenter);
    buttonLayout->addWidget(closeBtn);
    buttonLayout->setContentsMargins(0, 15, 0, 0); // 添加顶部间距
    mainLayout->addLayout(buttonLayout);

    // 连接关闭按钮信号
    connect(closeBtn, &QPushButton::clicked, scoreRankWindow, &QWidget::hide);
}

// 简单难度选择槽函数
void Widget::onEasySelected()
{
    difficultyWindow->hide();
    wni->setDifficulty(snake::Easy);
    wni->show();
}

// 普通难度选择槽函数
void Widget::onNormalSelected()
{
    difficultyWindow->hide();
    wni->setDifficulty(snake::Normal);
    wni->show();
}

// 困难难度选择槽函数
void Widget::onHardSelected()
{
    difficultyWindow->hide();
    wni->setDifficulty(snake::Hard);
    wni->show();
}

void Widget::setBtnclick() {
    // 重置设置窗口到当前设置
    
    // 设置地图选择
    if(currentMapIndex == 0) map1Radio->setChecked(true);
    else if(currentMapIndex == 1) map2Radio->setChecked(true);
    else if(currentMapIndex == 2) map3Radio->setChecked(true);
    updateMapPreview(currentMapIndex);
    
    // 设置皮肤选择
    if(currentSkinIndex == 0) skin1Radio->setChecked(true);
    else if(currentSkinIndex == 1) skin2Radio->setChecked(true);
    else if(currentSkinIndex == 2) skin3Radio->setChecked(true);
    updateSkinPreview(currentSkinIndex);
    
    // 居中显示设置窗口
    QPoint centerPos = this->geometry().center() - settingsWindow->rect().center();
    settingsWindow->move(centerPos);
    settingsWindow->show();
}

void Widget::rankBtnclick() {
    // 获取前十名分数
    QList<int> topTenScores = wni->GetTopTenScores();
    
    // 更新分数标签
    for(int i = 0; i < 10; ++i) {
        QLabel* scoreLabel = scoreRankWindow->findChild<QLabel*>(QString("scoreLabel_%1").arg(i));
        if(scoreLabel) {
            QString rankText;
            if(i == 0) {
                rankText = QString("🥇 第%1名：%2 分").arg(i + 1).arg(topTenScores[i]);
                scoreLabel->setStyleSheet(
                    "QLabel {"
                    "   padding: 12px;"
                    "   margin: 3px;"
                    "   border-radius: 8px;"
                    "   background-color: #f1c40f;"
                    "   color: #2c3e50;"
                    "   font-weight: bold;"
                    "   min-height: 45px;"
                    "}"
                );
            } else if(i == 1) {
                rankText = QString("🥈 第%1名：%2 分").arg(i + 1).arg(topTenScores[i]);
                scoreLabel->setStyleSheet(
                    "QLabel {"
                    "   padding: 12px;"
                    "   margin: 3px;"
                    "   border-radius: 8px;"
                    "   background-color: #bdc3c7;"
                    "   color: #2c3e50;"
                    "   font-weight: bold;"
                    "   min-height: 45px;"
                    "}"
                );
            } else if(i == 2) {
                rankText = QString("🥉 第%1名：%2 分").arg(i + 1).arg(topTenScores[i]);
                scoreLabel->setStyleSheet(
                    "QLabel {"
                    "   padding: 12px;"
                    "   margin: 3px;"
                    "   border-radius: 8px;"
                    "   background-color: #e67e22;"
                    "   color: white;"
                    "   font-weight: bold;"
                    "   min-height: 45px;"
                    "}"
                );
            } else {
                rankText = QString("      第%1名：%2 分").arg(i + 1).arg(topTenScores[i]);
                scoreLabel->setStyleSheet(
                    "QLabel {"
                    "   padding: 10px;"
                    "   margin: 3px;"
                    "   border-radius: 8px;"
                    "   background-color: #ecf0f1;"
                    "   color: #2c3e50;"
                    "   min-height: 45px;"
                    "}"
                );
            }
            scoreLabel->setText(rankText);
        }
    }
    
    // 居中显示分数排名窗口
    QPoint centerPos = this->geometry().center() - scoreRankWindow->rect().center();
    scoreRankWindow->move(centerPos);
    scoreRankWindow->show();
}

void Widget::quitBtnclick() {
    if(QMessageBox::Yes == QMessageBox::question(this, "提示",
        "确定要退出游戏吗？", QMessageBox::Yes|QMessageBox::No, QMessageBox::No)) {
        this->close();
        exit(0);
    } else {
        return;
    }
}

Widget::~Widget()
{
    delete ui;
}

void Widget::initSettingsWindow()
{
    // 创建设置窗口
    settingsWindow = new QWidget(this);
    settingsWindow->setWindowTitle("游戏设置");
    settingsWindow->setFixedSize(800, 700);
    settingsWindow->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    settingsWindow->hide(); // 初始隐藏

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(settingsWindow);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 添加标题
    QLabel* titleLabel = new QLabel("   游戏设置   ", settingsWindow);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont("Arial", 18, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setFixedHeight(60);
    titleLabel->setStyleSheet("color: #2c3e50; margin-bottom: 10px; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    // 1. 地图选择组
    QGroupBox* mapGroup = new QGroupBox("地图选择", settingsWindow);
    QHBoxLayout* mapMainLayout = new QHBoxLayout(mapGroup);
    
    // 地图选择布局
    QVBoxLayout* mapSelectLayout = new QVBoxLayout();
    mapButtonGroup = new QButtonGroup(settingsWindow);
    
    map1Radio = new QRadioButton("冰原", settingsWindow);
    map2Radio = new QRadioButton("草地", settingsWindow);
    map3Radio = new QRadioButton("沙漠", settingsWindow);
    
    map1Radio->setChecked(true);
    
    mapButtonGroup->addButton(map1Radio, 0);
    mapButtonGroup->addButton(map2Radio, 1);
    mapButtonGroup->addButton(map3Radio, 2);
    
    mapSelectLayout->addWidget(map1Radio);
    mapSelectLayout->addWidget(map2Radio);
    mapSelectLayout->addWidget(map3Radio);
    mapSelectLayout->addStretch();
    
    // 地图预览
    mapPreviewLabel = new QLabel(settingsWindow);
    mapPreviewLabel->setFixedSize(200, 150);
    mapPreviewLabel->setStyleSheet("border: 2px solid #3498db; border-radius: 8px; background-color: #ecf0f1;");
    mapPreviewLabel->setAlignment(Qt::AlignCenter);
    mapPreviewLabel->setText("地图预览");
    
    mapMainLayout->addLayout(mapSelectLayout);
    mapMainLayout->addWidget(mapPreviewLabel);
    mainLayout->addWidget(mapGroup);

    // 2. 皮肤选择组
    QGroupBox* skinGroup = new QGroupBox("蛇皮肤选择", settingsWindow);
    QHBoxLayout* skinMainLayout = new QHBoxLayout(skinGroup);
    
    // 皮肤选择布局
    QVBoxLayout* skinSelectLayout = new QVBoxLayout();
    skinButtonGroup = new QButtonGroup(settingsWindow);
    
    skin1Radio = new QRadioButton("小猫", settingsWindow);
    skin2Radio = new QRadioButton("团子", settingsWindow);
    skin3Radio = new QRadioButton("美西螈", settingsWindow);
    
    skin1Radio->setChecked(true);
    
    skinButtonGroup->addButton(skin1Radio, 0);
    skinButtonGroup->addButton(skin2Radio, 1);
    skinButtonGroup->addButton(skin3Radio, 2);
    
    skinSelectLayout->addWidget(skin1Radio);
    skinSelectLayout->addWidget(skin2Radio);
    skinSelectLayout->addWidget(skin3Radio);
    skinSelectLayout->addStretch();
    
    // 皮肤预览
    skinPreviewLabel = new QLabel(settingsWindow);
    skinPreviewLabel->setFixedSize(200, 150);
    skinPreviewLabel->setStyleSheet("border: 2px solid #e74c3c; border-radius: 8px; background-color: #ecf0f1;");
    skinPreviewLabel->setAlignment(Qt::AlignCenter);
    skinPreviewLabel->setText("皮肤预览");
    
    skinMainLayout->addLayout(skinSelectLayout);
    skinMainLayout->addWidget(skinPreviewLabel);
    mainLayout->addWidget(skinGroup);

    // 3. 按钮组
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setAlignment(Qt::AlignCenter);
    buttonLayout->setSpacing(20);
    
    QPushButton* confirmBtn = new QPushButton(" 确认", settingsWindow);
    QPushButton* cancelBtn = new QPushButton(" 取消", settingsWindow);
    
    confirmBtn->setFixedSize(100, 40);
    cancelBtn->setFixedSize(100, 40);
    
    QFont btnFont("Arial", 12, QFont::Bold);
    confirmBtn->setFont(btnFont);
    cancelBtn->setFont(btnFont);
    
    confirmBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #27ae60;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #229954;"
        "}"
    );
    
    cancelBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #e74c3c;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c0392b;"
        "}"
    );
    
    buttonLayout->addWidget(confirmBtn);
    buttonLayout->addWidget(cancelBtn);
    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(mapButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &Widget::onMapSelectionChanged);
    connect(skinButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &Widget::onSkinSelectionChanged);
    connect(confirmBtn, &QPushButton::clicked, this, &Widget::onSettingsConfirm);
    connect(cancelBtn, &QPushButton::clicked, this, &Widget::onSettingsCancel);
}

// 地图选择改变槽函数
void Widget::onMapSelectionChanged()
{
    int selectedMap = mapButtonGroup->checkedId();
    updateMapPreview(selectedMap);
}

// 皮肤选择改变槽函数
void Widget::onSkinSelectionChanged()
{
    int selectedSkin = skinButtonGroup->checkedId();
    updateSkinPreview(selectedSkin);
}

// 更新地图预览
void Widget::updateMapPreview(int mapIndex)
{
    QString mapName;
    QString previewText;
    QString styleSheet = "border: 2px solid #3498db; border-radius: 8px; padding: 10px; ";
    QString imagePath;
    
    switch(mapIndex) {
        case 0:
            mapName = "冰原";
            imagePath = ":/images/images/map1.png";
            break;
        case 1:
            mapName = "草地";
            imagePath = ":/images/images/map2.png";
            break;
        case 2:
            mapName = "沙漠";
            imagePath = ":/images/images/map3.png";
            break;
        default:
            imagePath = ":/images/images/map1.png";
    }
    
    // 加载并显示图片
    QPixmap mapPixmap(imagePath);
    if (!mapPixmap.isNull()) {
        // 缩放图片以适应预览标签的大小
        QPixmap scaledPixmap = mapPixmap.scaled(mapPreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        mapPreviewLabel->setPixmap(scaledPixmap);
    }
}

// 更新皮肤预览
void Widget::updateSkinPreview(int skinIndex)
{
    QString skinName;
    QString previewText;
    QString styleSheet = "border: 2px solid #e74c3c; border-radius: 8px; padding: 10px; ";
    QString headImagePath, bodyImagePath;
    
    switch(skinIndex) {
        case 0:
            skinName = "小猫";
            headImagePath = ":/images/images/SnakeHead1.png";
            break;
        case 1:
            skinName = "团子";
            headImagePath = ":/images/images/SnakeHead2.png";
            break;
        case 2:
            skinName = "美西螈";
            headImagePath = ":/images/images/SnakeHead3.png";
            break;
        default:
            skinName = "未知皮肤";
            headImagePath = ":/images/images/SnakeHead1.png";
    }
    
    // 尝试加载蛇头图片进行预览
    QPixmap headPixmap(headImagePath);
    if (!headPixmap.isNull()) {
        // 缩放图片以适应预览标签的大小
        QPixmap scaledPixmap = headPixmap.scaled(skinPreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        skinPreviewLabel->setPixmap(scaledPixmap);
    }
}

// 确认设置槽函数
void Widget::onSettingsConfirm()
{
    // 保存设置
    currentMapIndex = mapButtonGroup->checkedId();
    currentSkinIndex = skinButtonGroup->checkedId();
    
    // 将设置应用到游戏中
    wni->LoadGameImages(currentMapIndex, currentSkinIndex);
    
    QMessageBox::information(this, "设置", 
        QString("设置已保存！\n地图: %1\n皮肤: %2")
        .arg(currentMapIndex == 0 ? "冰原" : currentMapIndex == 1 ? "草地" : "沙漠")
        .arg(currentSkinIndex == 0 ? "小猫" : currentSkinIndex == 1 ? "团子" : "美西螈"));
    
    settingsWindow->hide();
}

// 取消设置槽函数
void Widget::onSettingsCancel()
{
    settingsWindow->hide();
}

// 自动演示槽函数
void Widget::autoBtnclick()
{
    // 启用AI并随机设置地图和皮肤
    autowni->enableAI(true);
    autowni->setDifficulty(snake::Hard);
    autowni->setRandomMapAndSkin();
    
    // 设置窗口标题和位置
    autowni->setWindowTitle("贪吃蛇 - 自动演示");
    autowni->setGeometry(100, 50, 1600, 900);
    
    // 显示演示窗口（不隐藏主菜单）
    autowni->show();
    autowni->InitSnake(); // 初始化游戏
}
