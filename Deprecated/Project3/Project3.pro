QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Project3
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11
CONFIG += console

SOURCES += \
    src/ecs/components.cpp \
    src/ecs/entity.cpp \
    src/main.cpp \
    src/globals.cpp \
    src/ecs/camera.cpp \
    src/ecs/scene.cpp  \
    src/input/input.cpp \
    src/input/interaction.cpp \
    src/input/selection.cpp \
    src/rendering/deferredrenderer.cpp \
    src/rendering/framebufferobject.cpp \
    src/rendering/gl.cpp \
    src/rendering/gldebug.cpp \
    src/resources/material.cpp \
    src/resources/mesh.cpp \
    src/resources/resource.cpp \
    src/resources/resourcemanager.cpp \
    src/resources/shaderprogram.cpp \
    src/resources/texture.cpp \
    src/resources/texturecube.cpp \
    src/ui/aboutopengldialog.cpp \
    src/ui/componentwidget.cpp \
    src/ui/DarkStyle.cpp \
    src/ui/entitywidget.cpp \
    src/ui/environmentwidget.cpp \
    src/ui/hierarchywidget.cpp \
    src/ui/inspectorwidget.cpp \
    src/ui/lightsourcewidget.cpp \
    src/ui/mainwindow.cpp \
    src/ui/materialwidget.cpp \
    src/ui/meshrendererwidget.cpp \
    src/ui/meshwidget.cpp \
    src/ui/miscsettingswidget.cpp \
    src/ui/openglwidget.cpp \
    src/ui/openglwidget_texture.cpp \
    src/ui/resourceswidget.cpp \
    src/ui/resourcewidget.cpp \
    src/ui/texturewidget.cpp \
    src/ui/toolswidget.cpp \
    src/ui/transformwidget.cpp \
    src/util/modelimporter.cpp \
    src/util/raycast.cpp

HEADERS += \
    src/ecs/components.h \
    src/ecs/entity.h \
    src/globals.h \
    src/ecs/camera.h \
    src/ecs/scene.h \
    src/input/input.h \
    src/input/interaction.h \
    src/input/selection.h \
    src/rendering/deferredrenderer.h \
    src/rendering/framebufferobject.h \
    src/rendering/gl.h \
    src/rendering/gldebug.h \
    src/resources/material.h \
    src/resources/mesh.h \
    src/resources/resource.h \
    src/resources/resourcemanager.h \
    src/resources/shaderprogram.h \
    src/resources/texture.h \
    src/resources/texturecube.h \
    src/ui/aboutopengldialog.h \
    src/ui/componentwidget.h \
    src/ui/DarkStyle.h \
    src/ui/entitywidget.h \
    src/ui/environmentwidget.h \
    src/ui/hierarchywidget.h \
    src/ui/inspectorwidget.h \
    src/ui/lightsourcewidget.h \
    src/ui/mainwindow.h \
    src/ui/materialwidget.h \
    src/ui/meshrendererwidget.h \
    src/ui/meshwidget.h \
    src/ui/miscsettingswidget.h \
    src/ui/openglwidget.h \
    src/ui/openglwidget_texture.h \
    src/ui/resourceswidget.h \
    src/ui/resourcewidget.h \
    src/ui/texturewidget.h \
    src/ui/toolswidget.h \
    src/ui/transformwidget.h \
    src/util/modelimporter.h \
    src/util/stb_image.h \
    src/util/raycast.h

FORMS += \
    ui/aboutopengldialog.ui \
    ui/componentwidget.ui \
    ui/entitywidget.ui \
    ui/hierarchywidget.ui \
    ui/mainwindow.ui \
    ui/materialwidget.ui \
    ui/meshwidget.ui \
    ui/miscsettingswidget.ui \
    ui/resourceswidget.ui \
    ui/resourcewidget.ui \
    ui/texturewidget.ui \
    ui/toolswidget.ui \
    ui/transformwidget.ui

INCLUDEPATH += src/

RESOURCES += \
    res/resources.qrc \
    res/darkstyle.qrc

DISTFILES += \
    res/forward_shading.frag \
    res/forward_shading.vert \
    res/texture_view.frag \
    res/texture_view.vert

# OpenGL
win32: LIBS += -lopengl32

# Assimp
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../ThirdParty/Assimp/lib/windows/ -lassimp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../ThirdParty/Assimp/lib/windows/ -lassimpd
else:unix: LIBS += -L$$PWD/../ThirdParty/Assimp/lib/osx/ -lassimp
INCLUDEPATH += $$PWD/../ThirdParty/Assimp/include
DEPENDPATH += $$PWD/../ThirdParty/Assimp/include
