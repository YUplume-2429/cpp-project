#include "gamewindow.h"
#include "ui_gamewindow.h"
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QIcon>
#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>

GameWindow::GameWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GameWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("选择游戏"));
    resize(900, 600);

    QWidget *central = new QWidget(this);
    QGridLayout *grid = new QGridLayout(central);
    grid->setHorizontalSpacing(40);
    grid->setVerticalSpacing(40);
    grid->setContentsMargins(40, 40, 40, 40);

    // 创建 2x3 的按钮 + 标签
    for (int i = 0; i < 6; ++i) {
        int r = i / 3;
        int c = i % 3;

        QWidget *cell = new QWidget(central);
        QVBoxLayout *v = new QVBoxLayout(cell);
        v->setSpacing(12);

        QPushButton *iconBtn = new QPushButton(cell);
        iconBtn->setFixedSize(150, 150);
        iconBtn->setObjectName(QString("gameBtn_%1").arg(i));
        iconBtn->setText(tr("游戏\n图标"));
        iconBtn->setIconSize(QSize(140, 140));

        QLabel *nameLabel = new QLabel(tr("游戏名称"), cell);
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setObjectName(QString("gameName_%1").arg(i));

        v->addWidget(iconBtn, 0, Qt::AlignHCenter);
        v->addWidget(nameLabel);
        grid->addWidget(cell, r, c);

        // 连接点击
        QObject::connect(iconBtn, &QPushButton::clicked, this, [this, i]() { onGameClicked(i); });
    }

    // 将网格布局作为顶层
    this->setLayout(grid);
}

GameWindow::~GameWindow()
{
    delete ui;
}

void GameWindow::setGameInfo(int index, const QString &name, const QString &iconPath, const QString &exePath)
{
    if (index < 0 || index >= 6) return;
    m_items[index].name = name;
    m_items[index].iconPath = iconPath;
    m_items[index].exePath = exePath;

    // 更新 UI
    auto btn = this->findChild<QPushButton*>(QString("gameBtn_%1").arg(index));
    auto label = this->findChild<QLabel*>(QString("gameName_%1").arg(index));
    if (label) label->setText(name.isEmpty() ? tr("游戏名称") : name);
    if (btn && !iconPath.isEmpty()) {
        QIcon ico(iconPath);
        if (!ico.isNull()) {
            btn->setIcon(ico);
            btn->setText("");
        }
    }
}

void GameWindow::onGameClicked(int index)
{
    if (index < 0 || index >= 6) return;
    const QString exe = m_items[index].exePath;
    if (exe.isEmpty()) {
        QMessageBox::warning(this, tr("未配置"), tr("未配置此游戏的可执行文件路径。"));
        return;
    }
    if (exe.startsWith(":/")) {
        QMessageBox::warning(this, tr("路径不支持"), tr("请使用文件系统路径而不是资源路径来启动外部 exe。"));
        return;
    }
    QFileInfo fi(exe);
    QString pathToRun = exe;
    if (!fi.isAbsolute()) {
        // 同级文件夹下：将相对路径基于可执行程序所在目录
        pathToRun = QCoreApplication::applicationDirPath() + "/" + exe;
    }
    if (!QFileInfo::exists(pathToRun)) {
        QMessageBox::warning(this, tr("文件不存在"), tr("找不到可执行文件: %1").arg(pathToRun));
        return;
    }

    // 运行exe（不阻塞本窗口），设置工作目录为exe所在目录以便加载其依赖
    const QString workDir = QFileInfo(pathToRun).absolutePath();
    bool ok = QProcess::startDetached(pathToRun, {}, workDir);
    if (!ok) {
        QMessageBox::critical(this, tr("启动失败"), tr("无法启动: %1").arg(pathToRun));
    }
}
