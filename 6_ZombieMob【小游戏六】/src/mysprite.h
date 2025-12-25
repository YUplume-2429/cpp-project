#pragma once

#include <QGraphicsPixmapItem>
#include <QVector>
#include <QElapsedTimer>
#include <QRectF>
#include <QPixmap>
#include "utils.h"

class MySprite : public QGraphicsPixmapItem {
public:
    MySprite(QGraphicsItem* parent = nullptr);

    // Load a spritesheet and slice into frames
    bool load(const QString& filename, int width, int height, int columns);

    // Update animation frames; current_time in ms
    void updateFrame(qint64 current_time_ms, int rate_ms = 30);

    // Frame/animation state (mirrors Python fields)
    int frame {0};
    int old_frame {-1};
    int frame_width {1};
    int frame_height {1};
    int first_frame {0};
    int last_frame {0};
    int columns {1};
    qint64 last_time {0};
    int direction {0}; // 0=N, 2=E, 4=S, 6=W
    Point velocity {0.f, 0.f};

    // Position helpers aligned with Python's X/Y based on top-left
    float X() const { return static_cast<float>(pos().x()); }
    float Y() const { return static_cast<float>(pos().y()); }
    void setXf(float v) { setPos(v, Y()); }
    void setYf(float v) { setPos(X(), v); }
    void setPosition(float x, float y) { setPos(x, y); }

    // Bounding rect scaled around center by ratio, for collide_rect_ratio
    QRectF scaledBoundingRect(double ratio) const;

private:
    QPixmap master_image_;
    QVector<QPixmap> frames_;
    void rebuildCurrentPixmap();
};
