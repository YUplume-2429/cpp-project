#include "gamewindow.h"
#include "assets.h"
#include "utils.h"

#include <QRandomGenerator>
#include <QApplication>
#include <QDebug>

namespace {
constexpr int kScreenW = 800;
constexpr int kScreenH = 600;
constexpr int kClampW = 700; // to match Python logic
constexpr int kClampH = 500;
}

GameWindow::GameWindow(QWidget* parent)
    : QGraphicsView(parent) {
    setWindowTitle("Collision Demo");
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    scene_ = new QGraphicsScene(0, 0, kScreenW, kScreenH, this);
    scene_->setBackgroundBrush(QColor(50, 50, 100));
    setScene(scene_);
    setFixedSize(kScreenW, kScreenH);

    // Create player
    player_ = new MySprite();
    player_->load(Assets::PLAYER_SPRITESHEET, 96, 96, 8);
    player_->setPosition(80, 80);
    player_->direction = 4;
    scene_->addItem(player_);

    // Create zombies
    for (int i = 0; i < 10; ++i) {
        auto* z = new MySprite();
        z->load(Assets::ZOMBIE_SPRITESHEET, 96, 96, 8);
        float zx = QRandomGenerator::global()->bounded(0, 701);
        float zy = QRandomGenerator::global()->bounded(0, 501);
        z->setPosition(zx, zy);
        int dirs[4] = {0, 2, 4, 6};
        z->direction = dirs[QRandomGenerator::global()->bounded(0, 4)];
        scene_->addItem(z);
        zombies_.push_back(z);
    }

    // Health item
    health_ = new MySprite();
    health_->load(Assets::HEALTH_IMAGE, 32, 32, 1);
    health_->setPosition(400, 300);
    scene_->addItem(health_);

    // Energy bar UI
    energy_fill_ = scene_->addRect(300, 570, player_health_ * 2, 25, QPen(Qt::NoPen), QBrush(QColor(50, 150, 50, 180)));
    energy_frame_ = scene_->addRect(300, 570, 200, 25, QPen(QColor(100, 200, 100, 180)));
    game_over_text_ = scene_->addSimpleText("G A M E   O V E R");
    game_over_text_->setBrush(QBrush(Qt::white));
    game_over_text_->setPos(300, 100);
    game_over_text_->setVisible(false);

    elapsed_.start();
    connect(&timer_, &QTimer::timeout, this, &GameWindow::onTick);
    timer_.start(1000 / 30); // 30 FPS
}

void GameWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        QApplication::quit();
        return;
    }
    if (event->isAutoRepeat()) return;
    switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_W: key_up_ = true; break;
        case Qt::Key_Right:
        case Qt::Key_D: key_right_ = true; break;
        case Qt::Key_Down:
        case Qt::Key_S: key_down_ = true; break;
        case Qt::Key_Left:
        case Qt::Key_A: key_left_ = true; break;
        default: break;
    }
}

void GameWindow::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;
    switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_W: key_up_ = false; break;
        case Qt::Key_Right:
        case Qt::Key_D: key_right_ = false; break;
        case Qt::Key_Down:
        case Qt::Key_S: key_down_ = false; break;
        case Qt::Key_Left:
        case Qt::Key_A: key_left_ = false; break;
        default: break;
    }
}

void GameWindow::onTick() {
    qint64 ticks = elapsed_.elapsed();

    // Input to direction: replicate Python's elif chain priority: Up, Right, Down, Left
    if (!game_over_) {
        if (key_up_) {
            player_->direction = 0; player_moving_ = true;
        } else if (key_right_) {
            player_->direction = 2; player_moving_ = true;
        } else if (key_down_) {
            player_->direction = 4; player_moving_ = true;
        } else if (key_left_) {
            player_->direction = 6; player_moving_ = true;
        } else {
            player_moving_ = false;
        }

        updatePlayerAnimationRange();
        updatePlayerMovement();

        // Update player frames (rate 50 as in Python calls)
        player_->updateFrame(ticks, 50);

        // Zombies
        updateZombies(ticks);

        // Zombie collision with player
        MySprite* attacker = nullptr;
        for (auto* z : zombies_) {
            if (player_->collidesWithItem(z)) { attacker = z; break; }
        }
        if (attacker) {
            if (collideRectRatio(player_, attacker, 0.5)) {
                player_health_ -= 10;
                if (attacker->X() < player_->X()) attacker->setXf(attacker->X() - 10);
                else if (attacker->X() > player_->X()) attacker->setXf(attacker->X() + 10);
            } else {
                attacker = nullptr;
            }
        }

        // Health update and collision
        health_->updateFrame(ticks, 50);
        updateHealthCollision();
    }

    // Game over check
    if (player_health_ <= 0) {
        game_over_ = true;
        game_over_text_->setVisible(true);
    }

    // Update energy bar
    if (energy_fill_) {
        scene_->removeItem(energy_fill_);
        delete energy_fill_;
        energy_fill_ = scene_->addRect(300, 570, qMax(0, player_health_) * 2, 25, QPen(Qt::NoPen), QBrush(QColor(50, 150, 50, 180)));
    }
}

void GameWindow::updatePlayerAnimationRange() {
    player_->first_frame = player_->direction * player_->columns;
    player_->last_frame = player_->first_frame + player_->columns - 1;
    if (player_->frame < player_->first_frame) player_->frame = player_->first_frame;
    if (!player_moving_) {
        // stop animating: lock frame
        player_->frame = player_->first_frame = player_->last_frame;
    } else {
        // velocity calc
        player_->velocity = calcVelocity(player_->direction, 1.5f);
        player_->velocity.x *= 1.5f;
        player_->velocity.y *= 1.5f;
    }
}

void GameWindow::updatePlayerMovement() {
    if (!player_moving_) return;
    float nx = player_->X() + player_->velocity.x;
    float ny = player_->Y() + player_->velocity.y;
    if (nx < 0) nx = 0; else if (nx > kClampW) nx = kClampW;
    if (ny < 0) ny = 0; else if (ny > kClampH) ny = kClampH;
    player_->setPosition(nx, ny);
}

void GameWindow::updateZombies(qint64 ticks) {
    for (auto* z : zombies_) {
        z->first_frame = z->direction * z->columns;
        z->last_frame = z->first_frame + z->columns - 1;
        if (z->frame < z->first_frame) z->frame = z->first_frame;
        z->velocity = calcVelocity(z->direction, 1.0f);
        float zx = z->X() + z->velocity.x;
        float zy = z->Y() + z->velocity.y;
        z->setPosition(zx, zy);
        if (z->X() < 0 || z->X() > kClampW || z->Y() < 0 || z->Y() > kClampH) {
            reverseDirection(z);
        }
        z->updateFrame(ticks, 50);
    }
}

void GameWindow::updateHealthCollision() {
    if (collideRectRatio(player_, health_, 0.5)) {
        player_health_ += 30;
        if (player_health_ > 100) player_health_ = 100;
        float hx = QRandomGenerator::global()->bounded(0, 701);
        float hy = QRandomGenerator::global()->bounded(0, 501);
        health_->setPosition(hx, hy);
    }
}

bool GameWindow::collideRectRatio(const MySprite* a, const MySprite* b, double ratio) const {
    if (!a || !b) return false;
    QRectF ar = a->scaledBoundingRect(ratio);
    QRectF br = b->scaledBoundingRect(ratio);
    return ar.intersects(br);
}
