#ifndef SNAKE_H
#define SNAKE_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QTimer>
#include <QQueue>
#include <QList>
#include <QRect>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QSet>
#include <QPoint>

// 前向声明
class SnakeAI;

QT_BEGIN_NAMESPACE
namespace Ui { class snake; }
QT_END_NAMESPACE

class snake : public QMainWindow
{
    Q_OBJECT

public:

    snake(QWidget *parent = nullptr);
    ~snake();
    void paintEvent(QPaintEvent *event = nullptr);
    void InitSnake();
    void keyPressEvent(QKeyEvent *event);
    QRect CreateFood();
    QRect CreateBoom();
    QRect CreateSpeedUpItem();              //创建加速道具
    QRect CreateSlowDownItem();             //创建减速道具
    QRect CreateTimeLimitFood();            //创建限时食物
    void CheckItemCollision();              //检查道具碰撞
    void UpdateItemEffects();               //更新道具效果

    // 道具效果相关
    enum SpeedEffectType {
        NO_EFFECT,      // 无效果
        SPEED_UP,       // 加速
        SLOW_DOWN       // 减速
    };
    void ApplySpeedEffect(SpeedEffectType effectType); //应用速度效果

    void ClearSpeedEffect();                //清除当前速度效果
    void GenerateRandomItems();             //随机生成道具
    bool IsPositionOccupied(const QRect& rect); //检查位置是否被占用
    void IsEat();
    void IsHit();
    void GameOver(const QString& message);   //统一的游戏结束处理
    void RestartGame();                      //重新开始游戏
    void PauseGame();                        //暂停游戏
    void ResumeGame();                       //恢复游戏
    void TogglePause();                      //切换暂停状态
    void LoadPauseImages();                  //加载暂停相关图片
    void LoadGameImages();                   //加载游戏图片资源
    void LoadGameImages(int mapIndex, int skinIndex); //带设置参数的加载函数
    
    QRect CreateRandomPosition();            //创建随机位置（统一函数）
    
    // 位置管理优化函数
    void UpdateOccupiedPositions();          //更新占用位置集合
    void AddOccupiedPosition(const QRect& rect); //添加占用位置
    void RemoveOccupiedPosition(const QRect& rect); //移除占用位置
    bool IsPositionOccupiedFast(const QPoint& gridPos); //快速检查网格位置是否被占用
    QPoint RectToGridPosition(const QRect& rect); //将QRect转换为网格坐标
    QRect GridPositionToRect(const QPoint& gridPos); //将网格坐标转换为QRect
    
    void MoveSnakeHead(QRect& head, int direction); //移动蛇头（简化方向逻辑）
    void DrawInfoPanel(QPainter& painter);   //绘制信息面板
    void DrawGameElements(QPainter& painter);//绘制游戏元素
    double calcActualScore(int score);       //计算难度补正后分数
    void LoadHighScores();                   //加载历史分数
    void SaveHighScore(int score);           //保存历史分数
    int GetHighestScore();                   //获取最高分
    QList<int> GetTopTenScores();            //获取前十名分数
    
    // AI相关函数
    void enableAI(bool enable = true);       //启用/禁用AI
    void setRandomMapAndSkin();              //随机设置地图和皮肤
    
    // 公开成员变量供AI访问
    QQueue<QRect> vSnakeRect;               //蛇容器
    QRect vSnakeHead;                       //蛇头
    QRect Food;                             //食物
    QRect SpeedUpItem;                      //加速道具
    QRect SlowDownItem;                     //减速道具
    QRect TimeLimitFood;                    //限时食物
    QList<QRect> Booms;                     //炸弹列表
    int currentDirection;                   //当前移动方向
    bool showTimeLimitFood;                 //是否显示限时食物
    int timeLimitCounter;                   //限时食物计数器(100格后消失)

    //新添加的难度
    enum Difficulty {
            Easy,
            Normal,
            Hard
        };
    void setDifficulty(Difficulty level);

public slots:
    void vSnake_update();

private:
    Ui::snake *ui;
    QTimer *timer;                          //更新定时器
    bool blsRun;                            //是否开始
    bool blsOver;                           //是否结束
    bool blsPaused;                         //是否暂停
    int newDirection;                       //蛇的将要移动方向
    QString Display;                        //字体显示
    int Score;                              //得分
    double score_with_dif;                  //难度补正后分数
    int Speed;                              //速度
    int originalSpeed;                      //原始速度(用于道具效果结束后恢复)
    QString diflevel;                       //难度等级传递
    
    // 快速检查位置占用
    QSet<QPoint> occupiedPositions;         //被占用的网格坐标集合
    
    // AI相关
    SnakeAI* aiController;                  //AI控制器
    bool aiEnabled;                         //是否启用AI
    
    // 缓存的图片资源 - 避免重复加载
    QPixmap mapImage;                       //地图图片
    QPixmap bgImage;                        //背景图片
    QPixmap foodImage;                      //食物图片
    QPixmap speedUpImage;                   //加速道具图片
    QPixmap speedDownImage;                 //减速道具图片
    QPixmap goldenFoodImage;                //黄金食物图片
    QPixmap barrierImage;                   //障碍物图片
    QPixmap snakeHeadImage;                 //蛇头图片
    QPixmap snakeBodyImage;                 //蛇身图片
    
    // 游戏常量
    static const int GRID_SIZE = 20;        //格子大小
    static const int GAME_AREA_X = 460;     //游戏区域X坐标
    static const int GAME_AREA_Y = 160;     //游戏区域Y坐标
    static const int GAME_AREA_WIDTH = 680; //游戏区域宽度
    static const int GAME_AREA_HEIGHT = 680;//游戏区域高度
    

    // 文字展示相关
    int displayTimeCounter;                 //文字显示计数器

    SpeedEffectType currentSpeedEffect;     //当前速度效果类型
    int speedEffectCounter;                 //速度效果计数器(100格后失效)
    int baseSpeedBeforeEffect;              //应用效果前的基准速度
    
    // 历史分数相关
    QList<int> highScores;                  //历史分数列表
    int highestScore;                       //历史最高分
    QString scoreFilePath;                  //分数文件路径
    
    // 暂停相关
    QPixmap pauseBackground;                //暂停背景图片
    QPixmap pauseIcon;                      //暂停图标
    QPixmap resumeIcon;                     //继续图标
};

#endif // snake_H
