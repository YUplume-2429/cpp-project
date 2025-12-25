#include "recordsdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDateTime>

RecordsDialog::RecordsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("继续游戏记录");
    resize(400, 500);
    auto* layout = new QVBoxLayout(this);
    auto* label = new QLabel("最近10次游玩记录", this);
    label->setAlignment(Qt::AlignCenter);
    list_ = new QListWidget(this);
    closeBtn_ = new QPushButton("关闭", this);
    layout->addWidget(label);
    layout->addWidget(list_);
    layout->addWidget(closeBtn_);
    connect(list_, &QListWidget::itemActivated, this, &RecordsDialog::onItemActivated);
    connect(closeBtn_, &QPushButton::clicked, this, &QDialog::reject);
}

void RecordsDialog::setRecords(const QList<GameRecord>& records) {
    list_->clear();
    if (records.isEmpty()) {
        auto* item = new QListWidgetItem("没有游玩记录");
        item->setFlags(Qt::NoItemFlags);
        list_->addItem(item);
        return;
    }
    for (const auto& rec : records) {
        auto* item = new QListWidgetItem(rec.title);
        item->setData(Qt::UserRole, QVariant::fromValue(rec.path));
        item->setData(Qt::UserRole + 1, rec.exists);
        list_->addItem(item);
    }
}

void RecordsDialog::onItemActivated(QListWidgetItem* item) {
    if (!item) return;
    bool exists = item->data(Qt::UserRole + 1).toBool();
    GameRecord rec;
    rec.title = item->text();
    rec.path = item->data(Qt::UserRole).toString();
    rec.exists = exists;
    emit recordChosen(rec);
    // 选择后立即关闭对话框，避免双击触发重复弹窗
    accept();
}