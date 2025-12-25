#include <QApplication>
#include "tankbattle.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    TankBattle w;
    w.showMenu(); // 显示菜单提示
    w.show();
    return a.exec();
}
