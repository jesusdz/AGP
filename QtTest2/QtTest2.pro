QT       += core gui
QT       += opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32: LIBS += -lopengl32

TARGET = QtTest2
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11
CONFIG += console

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    myopenglwidget.cpp \
    mycustomwidget.cpp \
    inspectorwidget.cpp \
    hierarchywidget.cpp \
    transformwidget.cpp \
    meshrendererwidget.cpp \
    componentwidget.cpp \
    scene.cpp

HEADERS += \
        mainwindow.h \
    myopenglwidget.h \
    mycustomwidget.h \
    inspectorwidget.h \
    hierarchywidget.h \
    transformwidget.h \
    meshrendererwidget.h \
    componentwidget.h \
    scene.h

FORMS += \
        mainwindow.ui \
    hierarchywidget.ui \
    transformwidget.ui \
    componentwidget.ui

RESOURCES += \
    icons.qrc
