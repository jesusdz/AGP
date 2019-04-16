QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
    src/globals.cpp \
    src/resources/material.cpp \
    src/resources/texture.cpp \
    src/resources/texturecube.cpp \
    src/ui/resourceswidget.cpp \
    src/ui/meshwidget.cpp \
    src/ui/resourcewidget.cpp \
    src/ui/openglwidget_texture.cpp \
    src/ui/texturewidget.cpp \
    src/ui/materialwidget.cpp \
    src/ui/lightsourcewidget.cpp \
    src/ui/environmentwidget.cpp \
    src/util/modelimporter.cpp \
    src/resources/shaderprogram.cpp \
    src/rendering/renderer.cpp \
    src/rendering/forwardrenderer.cpp \
    src/ui/input.cpp \
    src/ui/interaction.cpp \
    src/ecs/camera.cpp \
    src/ui/miscsettingswidget.cpp \
    src/rendering/framebufferobject.cpp \
    src/rendering/gl.cpp \
    src/rendering/gl.cpp \
    src/rendering/deferredrenderer.cpp \
    src/util/raycast.cpp \
    src/ui/selection.cpp

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
    src/globals.h \
    src/resources/material.h \
    src/resources/texture.h \
    src/resources/texturecube.h \
    src/ui/resourceswidget.h \
    src/ui/meshwidget.h \
    src/ui/resourcewidget.h \
    src/ui/openglwidget_texture.h \
    src/ui/texturewidget.h \
    src/ui/materialwidget.h \
    src/ui/lightsourcewidget.h \
    src/ui/environmentwidget.h \
    src/util/modelimporter.h \
    src/resources/shaderprogram.h \
    src/rendering/renderer.h \
    src/rendering/forwardrenderer.h \
    src/ui/input.h \
    src/ui/interaction.h \
    src/ecs/camera.h \
    src/ui/miscsettingswidget.h \
    src/rendering/framebufferobject.h \
    src/rendering/gl.h \
    src/rendering/deferredrenderer.h \
    src/util/raycast.h \
    src/util/stb_image.h \
    src/ui/selection.h

FORMS += \
    ui/mainwindow.ui \
    ui/hierarchywidget.ui \
    ui/transformwidget.ui \
    ui/componentwidget.ui \
    ui/entitywidget.ui \
    ui/aboutopengldialog.ui \
    ui/resourceswidget.ui \
    ui/meshwidget.ui \
    ui/resourcewidget.ui \
    ui/texturewidget.ui \
    ui/materialwidget.ui \
    ui/miscsettingswidget.ui

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
