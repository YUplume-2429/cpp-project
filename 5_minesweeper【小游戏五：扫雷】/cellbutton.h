#ifndef CELLBUTTON_H
#define CELLBUTTON_H

#include <QPushButton>

class CellButton : public QPushButton {
    Q_OBJECT
public:
    explicit CellButton(int row, int col, QWidget* parent = nullptr);

    int row() const { return row_; }
    int col() const { return col_; }

    bool isRevealed() const { return revealed_; }
    void setRevealed(bool r);

    bool isFlagged() const { return flagged_; }
    void setFlagged(bool f);

signals:
    void leftClicked(int row, int col);
    void rightClicked(int row, int col);

protected:
    void mousePressEvent(QMouseEvent* e) override;

private:
    int row_;
    int col_;
    bool revealed_ = false;
    bool flagged_ = false;
};

#endif // CELLBUTTON_H
