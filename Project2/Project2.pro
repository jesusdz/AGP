QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Project2
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11
CONFIG += console

SOURCES += \
src/main.cpp \
src/mainwindow.cpp \
src/inspectorwidget.cpp \
src/hierarchywidget.cpp \
src/transformwidget.cpp \
src/componentwidget.cpp \
src/scene.cpp \
src/entitywidget.cpp \
src/meshrendererwidget.cpp \
src/openglwidget.cpp

HEADERS += \
src/mainwindow.h \
src/inspectorwidget.h \
src/hierarchywidget.h \
src/transformwidget.h \
src/componentwidget.h \
src/scene.h \
src/entitywidget.h \
src/meshrendererwidget.h \
src/openglwidget.h

FORMS += \
ui/mainwindow.ui \
ui/hierarchywidget.ui \
ui/transformwidget.ui \
ui/componentwidget.ui \
ui/entitywidget.ui

RESOURCES += \
res/icons.qrc

LIBS += -lopengl32

