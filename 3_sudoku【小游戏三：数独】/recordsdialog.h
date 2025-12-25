#ifndef RECORDSDIALOG_H
#define RECORDSDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>

struct GameRecord {
    QString title; // 显示标题：时间与难度
    QString path;  // 记录文件路径或目录
    bool exists;   // 是否存在可继续的棋盘状态
};

class RecordsDialog : public QDialog {
    Q_OBJECT
public:
    explicit RecordsDialog(QWidget* parent = nullptr);
    void setRecords(const QList<GameRecord>& records);

signals:
    void recordChosen(const GameRecord& record);

private slots:
    void onItemActivated(QListWidgetItem* item);

private:
    QListWidget* list_;
    QPushButton* closeBtn_;
};

#endif // RECORDSDIALOG_H