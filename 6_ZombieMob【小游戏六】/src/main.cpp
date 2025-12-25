#include <QApplication>
#include "gamewindow.h"

#ifdef _WIN32
#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    GameWindow w;
    w.show();
    return app.exec();
}
