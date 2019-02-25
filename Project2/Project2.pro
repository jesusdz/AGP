QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -lopengl32

TARGET = Project2
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11
CONFIG += console

SOURCES += \
    src/main.cpp \
    src/ecs/scene.cpp  \
    src/ui/mainwindow.cpp \
    src/ui/inspectorwidget.cpp \
    src/ui/hierarchywidget.cpp \
    src/ui/transformwidget.cpp \
    src/ui/componentwidget.cpp \
    src/ui/entitywidget.cpp \
    src/ui/meshrendererwidget.cpp \
    src/ui/openglwidget.cpp \
    src/ui/aboutopengldialog.cpp \
    src/ui/DarkStyle.cpp \
    src/resources/mesh.cpp \
    src/resources/resource.cpp \
    src/resources/resourcemanager.cpp \
    src/globals.cpp

HEADERS += \
    src/ecs/scene.h \
    src/ui/mainwindow.h \
    src/ui/inspectorwidget.h \
    src/ui/hierarchywidget.h \
    src/ui/transformwidget.h \
    src/ui/componentwidget.h \
    src/ui/entitywidget.h \
    src/ui/meshrendererwidget.h \
    src/ui/openglwidget.h \
    src/ui/aboutopengldialog.h \
    src/ui/DarkStyle.h \
    src/resources/mesh.h \
    src/resources/resource.h \
    src/resources/resourcemanager.h \
    src/opengl/functions.h \
    src/globals.h

FORMS += \
    ui/mainwindow.ui \
    ui/hierarchywidget.ui \
    ui/transformwidget.ui \
    ui/componentwidget.ui \
    ui/entitywidget.ui \
    ui/aboutopengldialog.ui

INCLUDEPATH += src/

RESOURCES += \
    res/resources.qrc \
    res/darkstyle.qrc

DISTFILES += \
    res/shader1.vert \
    res/shader1.frag

