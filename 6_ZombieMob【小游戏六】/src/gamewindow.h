#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QKeyEvent>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include "mysprite.h"

class GameWindow : public QGraphicsView {
    Q_OBJECT
public:
    explicit GameWindow(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void onTick();

private:
    QGraphicsScene* scene_ {nullptr};
    QTimer timer_;
    QElapsedTimer elapsed_;

    // Sprites
    MySprite* player_ {nullptr};
    QVector<MySprite*> zombies_;
    MySprite* health_ {nullptr};

    // UI
    QGraphicsRectItem* energy_frame_ {nullptr};
    QGraphicsRectItem* energy_fill_ {nullptr};
    QGraphicsSimpleTextItem* game_over_text_ {nullptr};

    // State
    bool game_over_ {false};
    bool player_moving_ {false};
    int player_health_ {100};

    // Input state
    bool key_up_ {false};
    bool key_right_ {false};
    bool key_down_ {false};
    bool key_left_ {false};

    void updatePlayerAnimationRange();
    void updatePlayerMovement();
    void updateZombies(qint64 ticks);
    void updateHealthCollision();
    bool collideRectRatio(const MySprite* a, const MySprite* b, double ratio) const;
};
