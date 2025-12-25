#include "mysprite.h"
#include <QPainter>
#include <QDebug>

MySprite::MySprite(QGraphicsItem* parent) : QGraphicsPixmapItem(parent) {
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}

bool MySprite::load(const QString& filename, int width, int height, int cols) {
    if (!master_image_.load(filename)) {
        qWarning() << "Failed to load image:" << filename;
        return false;
    }
    frame_width = width;
    frame_height = height;
    columns = cols;
    int rows = master_image_.height() / height;
    int total_cols = master_image_.width() / width;
    if (columns <= 0) columns = qMax(1, total_cols);
    int total_frames = (master_image_.width() / width) * (master_image_.height() / height);
    last_frame = qMax(0, total_frames - 1);
    frames_.clear();
    frames_.reserve(total_frames);
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < total_cols; ++x) {
            QPixmap sub = master_image_.copy(x * width, y * height, width, height);
            frames_.push_back(sub);
        }
    }
    frame = 0;
    old_frame = -1;
    rebuildCurrentPixmap();
    setOffset(0, 0); // top-left alignment
    return true;
}

void MySprite::updateFrame(qint64 current_time_ms, int rate_ms) {
    if (current_time_ms > last_time + rate_ms) {
        frame += 1;
        if (frame > last_frame) {
            frame = first_frame;
        }
        last_time = current_time_ms;
    }
    if (frame != old_frame) {
        rebuildCurrentPixmap();
        old_frame = frame;
    }
}

QRectF MySprite::scaledBoundingRect(double ratio) const {
    QRectF r = boundingRect().translated(pos()); // scene rect equivalent for top-left
    QPointF c = r.center();
    QSizeF s = r.size() * ratio;
    QRectF out(QPointF(c.x() - s.width() / 2.0, c.y() - s.height() / 2.0), s);
    return out;
}

void MySprite::rebuildCurrentPixmap() {
    if (frames_.isEmpty()) return;
    int idx = qBound(0, frame, frames_.size() - 1);
    setPixmap(frames_[idx]);
}
