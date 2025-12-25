#include "difficultydialog.h"
#include <QVBoxLayout>
#include <QLabel>

DifficultyDialog::DifficultyDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("选择难度");
    resize(300, 200);
    auto* layout = new QVBoxLayout(this);
    auto* tip = new QLabel("\n简单≈线索44（较快）\n普通≈线索36（一般）\n困难≈线索28（较慢）", this);
    tip->setWordWrap(true);
    easyBtn_ = new QPushButton("简单", this);
    normalBtn_ = new QPushButton("普通", this);
    hardBtn_ = new QPushButton("困难", this);
    layout->addWidget(tip);
    layout->addWidget(easyBtn_);
    layout->addWidget(normalBtn_);
    layout->addWidget(hardBtn_);
    connect(easyBtn_, &QPushButton::clicked, this, &DifficultyDialog::chooseEasy);
    connect(normalBtn_, &QPushButton::clicked, this, &DifficultyDialog::chooseNormal);
    connect(hardBtn_, &QPushButton::clicked, this, &DifficultyDialog::chooseHard);
}

void DifficultyDialog::chooseEasy() { emit difficultyChosen(Difficulty::Easy); accept(); }
void DifficultyDialog::chooseNormal() { emit difficultyChosen(Difficulty::Normal); accept(); }
void DifficultyDialog::chooseHard() { emit difficultyChosen(Difficulty::Hard); accept(); }
