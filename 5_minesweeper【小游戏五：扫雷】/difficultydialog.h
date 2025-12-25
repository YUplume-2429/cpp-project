#ifndef DIFFICULTYDIALOG_H
#define DIFFICULTYDIALOG_H

#include <QDialog>

class DifficultyDialog : public QDialog {
    Q_OBJECT
public:
    explicit DifficultyDialog(QWidget* parent = nullptr);

    int rows() const { return rows_; }
    int cols() const { return cols_; }
    int mines() const { return mines_; }

private slots:
    void chooseEasy();
    void chooseMedium();
    void chooseHard();

private:
    void setDifficulty(int r, int c, int m);

    int rows_ = 9;
    int cols_ = 9;
    int mines_ = 10;
};

#endif // DIFFICULTYDIALOG_H
