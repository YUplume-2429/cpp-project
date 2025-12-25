#include "difficultydialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

DifficultyDialog::DifficultyDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("选择难度"));
    auto* layout = new QVBoxLayout(this);
    auto* label = new QLabel(QStringLiteral("请选择难度："), this);
    layout->addWidget(label);

    auto* easy = new QPushButton(QStringLiteral("简单 (9x9, 10雷)"), this);
    auto* medium = new QPushButton(QStringLiteral("普通 (16x16, 40雷)"), this);
    auto* hard = new QPushButton(QStringLiteral("困难 (30x16, 99雷)"), this);

    layout->addWidget(easy);
    layout->addWidget(medium);
    layout->addWidget(hard);

    connect(easy, &QPushButton::clicked, this, &DifficultyDialog::chooseEasy);
    connect(medium, &QPushButton::clicked, this, &DifficultyDialog::chooseMedium);
    connect(hard, &QPushButton::clicked, this, &DifficultyDialog::chooseHard);
}

void DifficultyDialog::setDifficulty(int r, int c, int m) {
    rows_ = r; cols_ = c; mines_ = m;
}

void DifficultyDialog::chooseEasy() {
    setDifficulty(9, 9, 10);
    accept();
}

void DifficultyDialog::chooseMedium() {
    setDifficulty(16, 16, 40);
    accept();
}

void DifficultyDialog::chooseHard() {
    setDifficulty(16, 30, 99); // 30x16
    accept();
}
