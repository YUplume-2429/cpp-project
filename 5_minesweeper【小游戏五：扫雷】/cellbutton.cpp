#include "cellbutton.h"

#include <QMouseEvent>
#include <QFont>

CellButton::CellButton(int row, int col, QWidget* parent)
    : QPushButton(parent), row_(row), col_(col) {
    setFixedSize(26, 26);
    setCheckable(false);
    QFont f = font();
    f.setBold(true);
    setFont(f);
}

void CellButton::setRevealed(bool r) {
    revealed_ = r;
    setEnabled(!r);
}

void CellButton::setFlagged(bool f) {
    flagged_ = f;
    setText(f ? QString::fromUtf8("\xF0\x9F\x9A\xA9") : ""); // 🚩
}

void CellButton::mousePressEvent(QMouseEvent* e) {
    if (revealed_) {
        e->ignore();
        return;
    }
    if (e->button() == Qt::RightButton) {
        emit rightClicked(row_, col_);
    } else if (e->button() == Qt::LeftButton) {
        emit leftClicked(row_, col_);
    }
    // Let QPushButton handle pressed visuals
    QPushButton::mousePressEvent(e);
}
