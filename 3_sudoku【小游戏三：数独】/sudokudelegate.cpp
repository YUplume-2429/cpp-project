#include "sudokudelegate.h"
#include <QPainter>

SudokuDelegate::SudokuDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void SudokuDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    // Default painting for text/background
    QStyledItemDelegate::paint(painter, option, index);

    const QRect r = option.rect;
    const int row = index.row();
    const int col = index.column();

    const int thin = 1;
    const int thick = 3;
    const QColor lineColor(80, 80, 80);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(QPen(lineColor, (row==0 || row==3 || row==6) ? thick : thin));
    painter->drawLine(r.topLeft(), QPoint(r.right(), r.top())); // top

    painter->setPen(QPen(lineColor, (col==0 || col==3 || col==6) ? thick : thin));
    painter->drawLine(r.topLeft(), QPoint(r.left(), r.bottom())); // left

    painter->setPen(QPen(lineColor, (col==2 || col==5 || col==8) ? thick : thin));
    painter->drawLine(QPoint(r.right(), r.top()), QPoint(r.right(), r.bottom())); // right

    painter->setPen(QPen(lineColor, (row==2 || row==5 || row==8) ? thick : thin));
    painter->drawLine(QPoint(r.left(), r.bottom()), QPoint(r.right(), r.bottom())); // bottom

    painter->restore();
}
