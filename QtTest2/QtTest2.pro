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
    inspector.cpp \
    myopenglwidget.cpp \
    transform.cpp \
    meshrenderer.cpp \
    customwidget.cpp

HEADERS += \
        mainwindow.h \
    inspector.h \
    myopenglwidget.h \
    transform.h \
    meshrenderer.h \
    customwidget.h

FORMS += \
        mainwindow.ui \
        rendering.ui \
    transform.ui
