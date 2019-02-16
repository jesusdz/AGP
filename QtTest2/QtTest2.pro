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
    meshrendererwidget.cpp

HEADERS += \
        mainwindow.h \
    myopenglwidget.h \
    mycustomwidget.h \
    inspectorwidget.h \
    hierarchywidget.h \
    transformwidget.h \
    meshrendererwidget.h

FORMS += \
        mainwindow.ui \
    hierarchywidget.ui \
    transformwidget.ui

RESOURCES += \
    icons.qrc
