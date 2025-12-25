#ifndef DIFFICULTYDIALOG_H
#define DIFFICULTYDIALOG_H

#include <QDialog>
#include <QPushButton>

enum class Difficulty { Easy, Normal, Hard };

class DifficultyDialog : public QDialog {
    Q_OBJECT
public:
    explicit DifficultyDialog(QWidget* parent = nullptr);

signals:
    void difficultyChosen(Difficulty diff);

private slots:
    void chooseEasy();
    void chooseNormal();
    void chooseHard();

private:
    QPushButton* easyBtn_;
    QPushButton* normalBtn_;
    QPushButton* hardBtn_;
};

#endif // DIFFICULTYDIALOG_H