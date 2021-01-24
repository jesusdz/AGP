QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Project1
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11
CONFIG += console

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    mycustomwidget.cpp \
    inspectorwidget.cpp \
    hierarchywidget.cpp \
    transformwidget.cpp \
    componentwidget.cpp \
    scene.cpp \
    entitywidget.cpp \
    shaperendererwidget.cpp \
    backgroundrendererwidget.cpp

HEADERS += \
        mainwindow.h \
    mycustomwidget.h \
    inspectorwidget.h \
    hierarchywidget.h \
    transformwidget.h \
    componentwidget.h \
    scene.h \
    entitywidget.h \
    shaperendererwidget.h \
    backgroundrendererwidget.h

FORMS += \
        mainwindow.ui \
    hierarchywidget.ui \
    transformwidget.ui \
    componentwidget.ui \
    entitywidget.ui

RESOURCES += \
    icons.qrc
