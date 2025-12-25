#include "snake.h"
#include "ui_snake.h"
#include "snake_ai.h"
#include <QPainter>
#include <QTimer>
#include <QRect>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QPixmap>
#include <QUrl>
#include <QDir>
#include <algorithm>

snake::snake(QWidget *parent):
    QMainWindow(parent),
    currentDirection(3),
    showTimeLimitFood(false),
    timeLimitCounter(0),
    ui(new Ui::snake),
    timer(nullptr),
    blsRun(false),
    blsOver(false),
    blsPaused(false),
    Score(0),
    Speed(300),
    originalSpeed(300),
    aiController(nullptr),
    aiEnabled(false),
    displayTimeCounter(100),
    currentSpeedEffect(NO_EFFECT),
    speedEffectCounter(0),
    baseSpeedBeforeEffect(300)
{
    ui->setupUi(this);
    this->setGeometry(QRect(150,40,1600,900));
    this->setFixedSize(1600, 900);
    this->setWindowTitle("贪吃蛇游戏");
    this->setFocusPolicy(Qt::StrongFocus); // 设置焦点策略以接收键盘事件
    
    // 初始化占用位置集合，预留一些空间提高性能
    occupiedPositions.reserve(1000);

    // 创建数据目录并初始化分数文件路径
//    QString dataDir = QCoreApplication::applicationDirPath() + "/data";
//    QDir().mkpath(dataDir); // 确保目录存在
//    scoreFilePath = dataDir + "/SnakeGame_HighScores.txt";
    scoreFilePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/SnakeGame_HighScores.txt";
    LoadHighScores(); // 加载历史分数
    
    // 加载游戏图片资源
    LoadGameImages();
    
    // 加载暂停相关图片
    LoadPauseImages();
}

snake::~snake()
{
    // 安全停止并删除定时器
    if(timer) {
        timer->stop();
        timer->deleteLater();
        timer = nullptr;
    }
    
    // 清理AI控制器
    if(aiController) {
        delete aiController;
        aiController = nullptr;
    }
    
    delete ui;
}


void snake::setDifficulty(snake::Difficulty level)
{
    Difficulty dif = level;

    // 根据难度设置蛇的移动速度
    switch(dif) {
    case Easy:
        Speed = originalSpeed = 300; // 简单难度：300ms
        diflevel = "简单";
        break;
    case Normal:
        Speed = originalSpeed = 200; // 普通难度：200ms
        diflevel = "普通";
        break;
    case Hard:
        Speed = originalSpeed = 100; // 困难难度：100ms
        diflevel = "困难";
        break;
    }
}

void snake::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    
    if(!blsRun){
        InitSnake();
    }

    //绘制背景图片
    painter.drawPixmap(QRect(0, 0, 1600, 900), bgImage);
    //绘制整体地图
    painter.drawPixmap(QRect(GAME_AREA_X, GAME_AREA_Y, GAME_AREA_WIDTH, GAME_AREA_HEIGHT), mapImage);

    // 绘制游戏状态文字
    QFont ft_Display = QFont("宋体", 12);
    ft_Display.setBold(true);
    painter.setFont(ft_Display);
    painter.setPen(Qt::red);
    painter.drawText(100, 800, Display);

    // 绘制游戏元素
    DrawGameElements(painter);

    // 绘制右侧信息面板
    DrawInfoPanel(painter);

    // 绘制暂停状态
    if(blsPaused) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 128));
        painter.drawRect(GAME_AREA_X, GAME_AREA_Y, GAME_AREA_WIDTH, GAME_AREA_HEIGHT);
        
        if(!pauseIcon.isNull()) {
            painter.drawPixmap(QRect(750, 450, 100, 100), pauseIcon);
        }
        
        painter.setFont(QFont("宋体", 36, QFont::Bold));
        painter.setPen(Qt::white);
        painter.drawText(QRect(GAME_AREA_X, GAME_AREA_Y, GAME_AREA_WIDTH, GAME_AREA_HEIGHT), 
                        Qt::AlignCenter, "\n游戏暂停\n\n\n按P键继续\n按空格键重新开始");
        return;
    }

    if (blsOver) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 128));
        painter.drawRect(GAME_AREA_X, GAME_AREA_Y, GAME_AREA_WIDTH, GAME_AREA_HEIGHT);
        painter.setFont(QFont("宋体", 36, QFont::Bold));
        painter.setPen(Qt::white);
        painter.drawText(QRect(GAME_AREA_X, GAME_AREA_Y, GAME_AREA_WIDTH, GAME_AREA_HEIGHT),
                        Qt::AlignCenter, "\nGame Over\n\n当前分数：" + QString::number(calcActualScore(Score)) + "\n\n按任意键重新开始");
        return;
    }

    if(blsOver && timer) {
        timer->stop();
    }
}

void snake::InitSnake(){
    blsRun = true;
    blsOver = false;
    blsPaused = false;
    currentDirection = newDirection = 3; // 初始向左移动
    Display = "游戏开始！";
    Food = CreateFood();
    Score = 0;

    // 重置道具状态
    SpeedUpItem = QRect();
    SlowDownItem = QRect();
    TimeLimitFood = QRect();
    Booms.clear();
    currentSpeedEffect = NO_EFFECT;
    speedEffectCounter = 0;
    baseSpeedBeforeEffect = originalSpeed;
    timeLimitCounter = 0;
    showTimeLimitFood = false;
    Speed = originalSpeed;
    
    // 初始化蛇
    vSnakeRect.clear();
    for(int i = 0; i < 3; i++) {
        QRect rect(760 + GRID_SIZE*i, 480, GRID_SIZE, GRID_SIZE);
        vSnakeRect.enqueue(rect);
    }

    // 安全的定时器管理
    if(timer) {
        timer->stop();
        timer->deleteLater();
    }
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &snake::vSnake_update);
    timer->start(Speed);
    
    // 初始化占用位置集合
    UpdateOccupiedPositions();
}
    
void snake::vSnake_update(){
    if(blsOver || blsPaused || vSnakeRect.isEmpty()) return;

    // 如果启用AI，获取AI的移动方向
    if(aiEnabled && aiController && vSnakeRect.size() >= 3) {
        int aiDirection = aiController->getNextDirection();
        // 防止反向移动
        if((currentDirection == 1 && aiDirection != 2) ||
           (currentDirection == 2 && aiDirection != 1) ||
           (currentDirection == 3 && aiDirection != 4) ||
           (currentDirection == 4 && aiDirection != 3) ||
           currentDirection == 0) {
            newDirection = aiDirection;
        }
    }

    currentDirection = newDirection;

    // 更新显示计数器
    if(--displayTimeCounter <= 0) {
        Display = "";
        displayTimeCounter = 100;
    }
    
    vSnakeHead = vSnakeRect.first();
    
    // 随机生成新道具和更新道具效果
    GenerateRandomItems();
    UpdateItemEffects();
    
    // 计算新蛇头位置
    QRect newHead = vSnakeHead;
    MoveSnakeHead(newHead, currentDirection);
    
    // 更新蛇头位置用于碰撞检测
    vSnakeHead = newHead;
    vSnakeRect.prepend(newHead);
    
    // 检查碰撞
    CheckItemCollision();
    if(blsOver) {
        update();
        return;
    }
    
    // 检查是否吃到食物
    bool hasEaten = (vSnakeHead == Food);
    if(hasEaten){
        Food = CreateFood();
        Score += 10;
        
        // 增加速度但不低于50ms
        if(currentSpeedEffect == NO_EFFECT) {
            // 没有速度效果时，直接修改当前速度
            if(Speed > 50){
                Speed -= 5;
                timer->start(Speed);
            }
        } else {
            // 有速度效果时，修改基准速度
            if(baseSpeedBeforeEffect > 50){
                baseSpeedBeforeEffect -= 5;
                // 重新应用当前效果以更新实际速度
                SpeedEffectType currentEffect = currentSpeedEffect;
                int remainingCounter = speedEffectCounter;
                ClearSpeedEffect();
                ApplySpeedEffect(currentEffect);
                speedEffectCounter = remainingCounter; // 保持剩余时间不变
            }
        }
    } else {
        // 没吃到食物，移除尾部
        if(vSnakeRect.size() > 1) {
            vSnakeRect.removeLast();
        }
    }
    
    // 检查自身碰撞
    IsHit();
    if(blsOver) {
        update();
        return;
    }

    // 检查边界碰撞
    if(vSnakeHead.left() < 480 || vSnakeHead.right() > 1120 ||
       vSnakeHead.top() < 180 || vSnakeHead.bottom() > 820) {
        GameOver("撞到墙壁，游戏结束！");
        return;
    }

    // 更新占用位置集合
    UpdateOccupiedPositions();
    
    update();
}


//1:UP, 2:DOWN, 3:LEFT, 4:RIGHT
void snake::keyPressEvent(QKeyEvent *event)
{
    if (!blsOver) {
        // 游戏进行中的按键处理
        switch(event->key()){
        case Qt::Key_W:
        case Qt::Key_Up:
            if(!blsRun || blsPaused) return;
            if(currentDirection != 2) newDirection = 1; // 防止反向
            break;
        case Qt::Key_S:
        case Qt::Key_Down:
            if(!blsRun || blsPaused) return;
            if(currentDirection != 1) newDirection = 2;
            break;
        case Qt::Key_A:
        case Qt::Key_Left:
            if(!blsRun || blsPaused) return;
            if(currentDirection != 4) newDirection = 3;
            break;
        case Qt::Key_D:
        case Qt::Key_Right:
            if(!blsRun || blsPaused) return;
            if(currentDirection != 3) newDirection = 4;
            break;
        case Qt::Key_Space: // 空格键重新开始
            if(blsPaused) {
                RestartGame(); // 暂停时重新开始
            }
            break;
        case Qt::Key_P: // P键暂停/继续
            if(blsRun) {
                TogglePause();
            }
            break;
        case Qt::Key_Escape: // ESC键暂停
            if(blsRun) {
                PauseGame();
            }
            break;
        default:
            break;
        }
    }
    else {
        // 游戏结束时，按任意键重新开始
        RestartGame();
    }
}

QRect snake::CreateFood(){
    return CreateRandomPosition();
}

QRect snake::CreateBoom(){
    return CreateRandomPosition();
}

QRect snake::CreateSpeedUpItem(){
    return CreateRandomPosition();
}

QRect snake::CreateSlowDownItem(){
    return CreateRandomPosition();
}

QRect snake::CreateTimeLimitFood(){
    return CreateRandomPosition();
}

void snake::IsEat(){
    // 这个函数现在主要用于检查碰撞，实际的吃食物逻辑已经移到vSnake_update中
}

void snake::IsHit(){
    // 检查是否撞到自己（跳过蛇头，检查蛇身）
    if(vSnakeRect.size() < 2) return; // 蛇身长度小于2不会自撞
    
    auto it = vSnakeRect.begin() + 1; // 跳过蛇头
    while(it != vSnakeRect.end()) {
        if(vSnakeHead == *it) {
            GameOver("撞到自己，游戏结束！");
            return;
        }
        ++it;
    }
}

// 检查位置是否被占用
bool snake::IsPositionOccupied(const QRect& rect){
    // 使用优化后的快速检查方法
    QPoint gridPos = RectToGridPosition(rect);
    return IsPositionOccupiedFast(gridPos);
}

// 快速检查网格位置是否被占用
bool snake::IsPositionOccupiedFast(const QPoint& gridPos){
    return occupiedPositions.contains(gridPos);
}

// 将QRect转换为网格坐标
QPoint snake::RectToGridPosition(const QRect& rect){
    // 计算相对于游戏区域的网格坐标
    int gridX = (rect.x() - 480) / GRID_SIZE;
    int gridY = (rect.y() - 180) / GRID_SIZE;
    return QPoint(gridX, gridY);
}

// 将网格坐标转换为QRect
QRect snake::GridPositionToRect(const QPoint& gridPos){
    int x = 480 + gridPos.x() * GRID_SIZE;
    int y = 180 + gridPos.y() * GRID_SIZE;
    return QRect(x, y, GRID_SIZE, GRID_SIZE);
}

// 添加占用位置
void snake::AddOccupiedPosition(const QRect& rect){
    QPoint gridPos = RectToGridPosition(rect);
    occupiedPositions.insert(gridPos);
}

// 移除占用位置
void snake::RemoveOccupiedPosition(const QRect& rect){
    QPoint gridPos = RectToGridPosition(rect);
    occupiedPositions.remove(gridPos);
}

// 更新占用位置集合 - 重建整个集合
void snake::UpdateOccupiedPositions(){
    occupiedPositions.clear();
    
    // 添加蛇身所有位置
    QQueue<QRect> tempQueue = vSnakeRect;
    while(!tempQueue.isEmpty()) {
        AddOccupiedPosition(tempQueue.dequeue());
    }
    
    // 添加食物位置
    if(!Food.isNull()) {
        AddOccupiedPosition(Food);
    }
    
    // 添加道具位置
    if(!SpeedUpItem.isNull()) {
        AddOccupiedPosition(SpeedUpItem);
    }
    if(!SlowDownItem.isNull()) {
        AddOccupiedPosition(SlowDownItem);
    }
    if(!TimeLimitFood.isNull()) {
        AddOccupiedPosition(TimeLimitFood);
    }
    
    // 添加炸弹位置
    for(const QRect& boom : Booms) {
        AddOccupiedPosition(boom);
    }
}

// 随机生成道具
void snake::GenerateRandomItems(){
    // 每次更新都有一定概率生成道具
    int random = QRandomGenerator::global()->bounded(0, 1000);
    
    // 0.5% 概率生成加速道具
    if(random < 5 && SpeedUpItem.isNull()) {
        SpeedUpItem = CreateSpeedUpItem();
    }
    
    // 0.5% 概率生成减速道具
    if(random >= 5 && random < 10 && SlowDownItem.isNull()) {
        SlowDownItem = CreateSlowDownItem();
    }
    
    // 0.3% 概率生成限时食物
    if(random >= 10 && random < 13 && !showTimeLimitFood) {
        TimeLimitFood = CreateTimeLimitFood();
        showTimeLimitFood = true;
        timeLimitCounter = 100; // 100格后消失
    }
    
    // 2% 概率生成炸弹，最多20个
    if(random >= 13 && random < 33 && Booms.size() < 20) {
        Booms.append(CreateBoom());
    }
}

// 检查道具碰撞
void snake::CheckItemCollision(){
    // 检查加速道具碰撞
    if(!SpeedUpItem.isNull() && vSnakeHead == SpeedUpItem) {
        SpeedUpItem = QRect();
        ApplySpeedEffect(SPEED_UP);
        Display = "加速！";
    }
    
    // 检查减速道具碰撞
    if(!SlowDownItem.isNull() && vSnakeHead == SlowDownItem) {
        SlowDownItem = QRect();
        ApplySpeedEffect(SLOW_DOWN);
        Display = "减速！";
    }
    
    // 检查限时食物碰撞
    if(showTimeLimitFood && !TimeLimitFood.isNull() && vSnakeHead == TimeLimitFood) {
        TimeLimitFood = QRect();
        showTimeLimitFood = false;
        timeLimitCounter = 0;
        Score += 50;
        Display = "限时食物 +50分！";
    }
    
    // 检查炸弹碰撞
    for(const QRect& boom : Booms) {
        if(vSnakeHead == boom) {
            GameOver("撞到砖块！游戏结束！");
            return;
        }
    }
}

// 更新道具效果
void snake::UpdateItemEffects(){
    // 更新速度效果计数器
    if(currentSpeedEffect != NO_EFFECT && --speedEffectCounter <= 0) {
        ClearSpeedEffect();
        Display = "速度效果结束";
    }
    
    // 更新限时食物计数器
    if(showTimeLimitFood && --timeLimitCounter <= 0) {
        TimeLimitFood = QRect();
        showTimeLimitFood = false;
    }
}

// 统一的游戏结束处理
void snake::GameOver(const QString& message){
    Display = message;
    
    blsOver = true;
    if(timer) timer->stop();
    
    // 保存分数
    if(Score > 0) {
        SaveHighScore(Score);
        // 更新最高分
        if(Score > highestScore) {
            highestScore = Score;
        }
    }
    
    update();
}

// 重新开始游戏
void snake::RestartGame(){
    blsRun = false;
    blsOver = false;
    blsPaused = false;
    update(); // 这会触发InitSnake()
}

// 计算补正后分数
double snake::calcActualScore(int score) {
    double scorecorrection = 1;
    if (diflevel == "普通") {
        scorecorrection = 1.1;
    } else if (diflevel == "困难") {
        scorecorrection = 1.2;
    }
    double score_with_dif = double(score) * scorecorrection;;
    return score_with_dif;
}


// 加载历史分数
void snake::LoadHighScores(){
    QFile file(scoreFilePath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        highScores.clear();
        
        while(!in.atEnd()) {
            QString line = in.readLine();
            bool ok;
            int score = line.toInt(&ok);
            if(ok && score > 0) {
                highScores.append(score);
            }
        }
        file.close();
        
        // 排序（降序）
        std::sort(highScores.begin(), highScores.end(), std::greater<int>());
        
        // 更新最高分
        highestScore = highScores.isEmpty() ? 0 : highScores.first();
    } else {
        // 文件不存在或无法打开，初始化为空
        highScores.clear();
        highestScore = 0;
    }
}

// 保存历史分数
void snake::SaveHighScore(int score){
    if(score <= 0) return;
    
    // 添加新分数到列表
    highScores.append(calcActualScore(score));
    
    // 排序（降序）
    std::sort(highScores.begin(), highScores.end(), std::greater<int>());
    
    // 只保留前50个最高分
    if(highScores.size() > 50) {
        highScores = highScores.mid(0, 50);
    }
    
    // 写入文件
    QFile file(scoreFilePath);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for(int i = 0; i < highScores.size(); ++i) {
            out << highScores[i] << "\n";
        }
        file.close();
    }
}

// 获取最高分
int snake::GetHighestScore(){
    return highestScore;
}

// 获取前十名分数
QList<int> snake::GetTopTenScores(){
    QList<int> topTen;
    
    // 确保分数列表是最新的
    LoadHighScores();
    
    // 取前10个分数，如果不足10个则用0补充
    for(int i = 0; i < 10; ++i) {
        if(i < highScores.size()) {
            topTen.append(highScores[i]);
        } else {
            topTen.append(0);
        }
    }
    
    return topTen;
}

// 暂停游戏
void snake::PauseGame(){
    if(!blsRun || blsOver || blsPaused) return;
    
    blsPaused = true;
    if(timer) timer->stop(); // 停止计时器
    Display = "游戏已暂停";
    update(); // 刷新界面
}

// 恢复游戏
void snake::ResumeGame(){
    if(!blsRun || blsOver || !blsPaused) return;
    
    blsPaused = false;
    if(timer) timer->start(Speed); // 重新启动计时器
    Display = "游戏继续";
    update(); // 刷新界面
}

// 切换暂停状态
void snake::TogglePause(){
    if(!blsRun || blsOver) return;
    
    if(blsPaused) {
        ResumeGame();
    } else {
        PauseGame();
    }
}

// 加载暂停相关图片
void snake::LoadPauseImages(){
    // 这里预留暂停图片的加载函数
    
    // 示例代码（需要在resources.qrc中添加对应的图片资源）：
    // pauseBackground = QPixmap(":/images/images/pause_background.png");
    // pauseIcon = QPixmap(":/images/images/pause_icon.png");
    // resumeIcon = QPixmap(":/images/images/resume_icon.png");
    
    // 如果没有图片资源，创建简单的几何图形
    pauseIcon = QPixmap(100, 100);
    pauseIcon.fill(Qt::transparent);
    QPainter iconPainter(&pauseIcon);
    iconPainter.setPen(QPen(Qt::white, 3));
    iconPainter.setBrush(Qt::white);
    
    // 绘制暂停符号（两个竖条）
    iconPainter.drawRect(25, 20, 15, 60);
    iconPainter.drawRect(60, 20, 15, 60);
    
    // 为继续图标创建播放符号（三角形）
    resumeIcon = QPixmap(100, 100);
    resumeIcon.fill(Qt::transparent);
    QPainter resumePainter(&resumeIcon);
    resumePainter.setPen(QPen(Qt::white, 3));
    resumePainter.setBrush(Qt::white);
    
    QPolygon triangle;
    triangle << QPoint(30, 20) << QPoint(30, 80) << QPoint(75, 50);
    resumePainter.drawPolygon(triangle);
}

// 加载游戏图片资源
void snake::LoadGameImages(){
    // 加载并缓存所有游戏图片，避免在paintEvent中重复加载
    mapImage = QPixmap(":/images/images/map1.png");
    bgImage = QPixmap(":/images/images/map1_background.png");
    foodImage = QPixmap(":/images/images/Food.png");
    speedUpImage = QPixmap(":/images/images/SpeedUp.jpg");
    speedDownImage = QPixmap(":/images/images/SpeedDown.jpg");
    goldenFoodImage = QPixmap(":/images/images/GoldenFood.png");
    barrierImage = QPixmap(":/images/images/Barrier.png");
    
    // 加载并缩放蛇的图片
    snakeHeadImage = QPixmap(":/images/images/SnakeHead1.png").scaled(GRID_SIZE, GRID_SIZE, Qt::KeepAspectRatio);
    snakeBodyImage = QPixmap(":/images/images/SnakeBody1.png").scaled(15, 15, Qt::KeepAspectRatio);
}

// 带设置参数的加载游戏图片资源函数
void snake::LoadGameImages(int mapIndex, int skinIndex){
    // 根据地图索引加载不同的地图
    switch(mapIndex) {
        case 0: // 冰原
            mapImage = QPixmap(":/images/images/map1.png");
            bgImage = QPixmap(":/images/images/map1_background.png");
            break;
        case 1: // 草地
            mapImage = QPixmap(":/images/images/map2.png");
            bgImage = QPixmap(":/images/images/map2_background.png");
            break;
        case 2: // 沙漠
            mapImage = QPixmap(":/images/images/map3.png");
            bgImage = QPixmap(":/images/images/map3_background.png");
            break;
        default:
            mapImage = QPixmap(":/images/images/map1.png");
    }
    
    // 根据皮肤索引加载不同的蛇头和蛇身
    QString headImagePath, bodyImagePath;
    switch(skinIndex) {
        case 0: // 小猫
            headImagePath = ":/images/images/SnakeHead1.png";
            bodyImagePath = ":/images/images/SnakeBody1.png";
            break;
        case 1: // 团子
            headImagePath = ":/images/images/SnakeHead2.png";
            bodyImagePath = ":/images/images/SnakeBody2.png";
            break;
        case 2: // 美西螈
            headImagePath = ":/images/images/SnakeHead3.png";
            bodyImagePath = ":/images/images/SnakeBody3.png";
            break;
        default:
            headImagePath = ":/images/images/SnakeHead1.png";
            bodyImagePath = ":/images/images/SnakeBody1.png";
    }
    
    // 加载并缩放蛇的图片
    snakeHeadImage = QPixmap(headImagePath).scaled(GRID_SIZE, GRID_SIZE, Qt::KeepAspectRatio);
    snakeBodyImage = QPixmap(bodyImagePath).scaled(15, 15, Qt::KeepAspectRatio);
    
    // 加载其他固定的游戏图片资源
    foodImage = QPixmap(":/images/images/Food.png");
    speedUpImage = QPixmap(":/images/images/SpeedUp.jpg");
    speedDownImage = QPixmap(":/images/images/SpeedDown.jpg");
    goldenFoodImage = QPixmap(":/images/images/GoldenFood.png");
    barrierImage = QPixmap(":/images/images/Barrier.png");
}

// 创建随机位置 - 优化版本
QRect snake::CreateRandomPosition(){
    QPoint gridPos;
    do {
        int x = QRandomGenerator::global()->bounded(1, 31);
        int y = QRandomGenerator::global()->bounded(1, 31);
        gridPos = QPoint(x, y);
    } while(IsPositionOccupiedFast(gridPos));
    
    return GridPositionToRect(gridPos);
}

// 移动蛇头
void snake::MoveSnakeHead(QRect& head, int direction){
    switch(direction){
    case 1: // 上
        head.translate(0, -GRID_SIZE);
        break;
    case 2: // 下
        head.translate(0, GRID_SIZE);
        break;
    case 3: // 左
        head.translate(-GRID_SIZE, 0);
        break;
    case 4: // 右
        head.translate(GRID_SIZE, 0);
        break;
    }
}

// 绘制信息面板
void snake::DrawInfoPanel(QPainter& painter){
    QFont infoFont("宋体", 17);
    painter.setFont(infoFont);
    painter.setPen(Qt::black);
    double scorecorrection = 1;
    if (diflevel == "普通") {
        scorecorrection = 1.1;
    } else if (diflevel == "困难") {
        scorecorrection = 1.2;
    }
    painter.drawText(1250, 40, "难度:" + diflevel + " 分数*" + QString::number(scorecorrection));
    painter.drawText(1250, 80, "历史最高分:" + QString::number(highestScore));
    painter.drawText(1250, 120, "目前得分: " + QString::number(calcActualScore(Score)));
    
    // 显示当前速度效果状态
    switch(currentSpeedEffect) {
    case SPEED_UP:
        painter.setPen(Qt::green);
        painter.drawText(1250, 160, "加速中(" + QString::number(speedEffectCounter) + ")");
        break;
    case SLOW_DOWN:
        painter.setPen(Qt::blue);
        painter.drawText(1250, 160, "减速中(" + QString::number(speedEffectCounter) + ")");
        break;
    case NO_EFFECT:
    default:
        // 无效果时不显示任何内容
        break;
    }
    
    if(showTimeLimitFood) {
        painter.setPen(Qt::yellow);
        painter.drawText(1250, 190, "限时食物(" + QString::number(timeLimitCounter) + ")");
    }
}

// 绘制游戏元素
void snake::DrawGameElements(QPainter& painter){
    // 绘制食物
    painter.drawPixmap(Food, foodImage);

    // 绘制加速道具
    if(!SpeedUpItem.isNull()) {
        painter.drawPixmap(SpeedUpItem, speedUpImage);
    }

    // 绘制减速道具
    if(!SlowDownItem.isNull()) {
        painter.drawPixmap(SlowDownItem, speedDownImage);
    }

    // 绘制限时食物
    if(showTimeLimitFood && !TimeLimitFood.isNull()) {
        painter.drawPixmap(TimeLimitFood, goldenFoodImage);
    }

    // 绘制炸弹
    for(const QRect& boom : Booms) {
        painter.drawPixmap(boom, barrierImage);
    }

    // 绘制蛇身（跳过蛇头）
    auto it = vSnakeRect.begin();
    if(it != vSnakeRect.end()) {
        ++it; // 跳过蛇头
        while(it != vSnakeRect.end()) {
            painter.drawPixmap(*it, snakeBodyImage);
            ++it;
        }
    }
    
    // 绘制蛇头（带旋转）
    if (!vSnakeRect.isEmpty()) {
        painter.save();
        QRect headRect = vSnakeRect.first();
        painter.translate(headRect.center());
        
        // 根据方向旋转
        switch(currentDirection){
        case 1: painter.rotate(0); break;    // 上
        case 2: painter.rotate(180); break;  // 下
        case 3: painter.rotate(270); break;  // 左
        case 4: painter.rotate(90); break;   // 右
        }
        
        painter.drawPixmap(QPoint(-headRect.width()/2, -headRect.height()/2), snakeHeadImage);
        painter.restore();
    }
}

// AI相关函数实现
void snake::enableAI(bool enable)
{
    aiEnabled = enable;
    if(enable && !aiController) {
        aiController = new SnakeAI(this);
    } else if(!enable && aiController) {
        delete aiController;
        aiController = nullptr;
    }
}

void snake::setRandomMapAndSkin()
{
    // 随机选择地图 (0-2)
    int randomMap = QRandomGenerator::global()->bounded(3);
    // 随机选择皮肤 (0-2) 
    int randomSkin = QRandomGenerator::global()->bounded(3);
    
    // 使用随机的地图和皮肤加载游戏资源
    LoadGameImages(randomMap, randomSkin); // 随机地图和皮肤
}

// 应用速度效果
void snake::ApplySpeedEffect(SpeedEffectType effectType)
{
    // 如果已有效果，先清除
    if(currentSpeedEffect != NO_EFFECT) {
        ClearSpeedEffect();
    }
    
    // 记录当前基准速度（应用效果前的速度）
    baseSpeedBeforeEffect = Speed;
    
    // 应用新效果
    currentSpeedEffect = effectType;
    speedEffectCounter = 100; // 100帧后效果消失
    
    switch(effectType) {
    case SPEED_UP: {
        // 加速：减少延迟时间，但不低于50ms
        int speedDecrease = qMin(50, Speed - 50);
        if(speedDecrease > 0) {
            Speed -= speedDecrease;
            timer->start(Speed);
        }
        break;
    }
    case SLOW_DOWN: {
        // 减速：增加延迟时间，但不超过500ms
        int speedIncrease = qMin(100, 500 - Speed);
        if(speedIncrease > 0) {
            Speed += speedIncrease;
            timer->start(Speed);
        }
        break;
    }
    case NO_EFFECT:
    default:
        // 无效果，不做任何处理
        break;
    }
}

// 清除当前速度效果
void snake::ClearSpeedEffect()
{
    if(currentSpeedEffect != NO_EFFECT) {
        // 恢复到应用效果前的基准速度
        Speed = baseSpeedBeforeEffect;
        
        // 重置效果状态
        currentSpeedEffect = NO_EFFECT;
        speedEffectCounter = 0;
        
        // 确保速度在合理范围内
        if(Speed < 50) Speed = 50;
        if(Speed > 500) Speed = 500;
        
        // 更新定时器
        timer->start(Speed);
    }
}
