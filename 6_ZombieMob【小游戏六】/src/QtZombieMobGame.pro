QT += widgets

TEMPLATE = app
TARGET = QtZombieMobGame

SOURCES += \
    main.cpp \
    gamewindow.cpp \
    mysprite.cpp \
    utils.cpp

HEADERS += \
    assets.h \
    gamewindow.h \
    mysprite.h \
    utils.h

# 资源文件（如有 .qrc）可在此添加
# RESOURCES += resources.qrc

# Windows 平台的字符集与警告等级（可选）
win32:QMAKE_CXXFLAGS += /utf-8
