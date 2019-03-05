#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/hierarchywidget.h"
#include "ui/resourceswidget.h"
#include "ui/inspectorwidget.h"
#include "ui/openglwidget.h"
#include "ui/aboutopengldialog.h"
#include "ecs/scene.h"
#include "resources/resourcemanager.h"
#include "resources/mesh.h"
#include "resources/texture.h"
#include "resources/material.h"
#include "globals.h"
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QJsonDocument>
#include <QJsonObject>


MainWindow *g_MainWindow = nullptr;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    uiMainWindow(new Ui::MainWindow)
{
    // In globals.h / globals.cpp
    resourceManager = new ResourceManager();
    scene = new Scene();

    g_MainWindow = this;
    uiMainWindow->setupUi(this);

    // All tab positions on top of the docking area
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::TabPosition::North);

    // Create the hierarchy widget and add it to the inspector dock
    hierarchyWidget = new HierarchyWidget();
    uiMainWindow->hierarchyDock->setWidget(hierarchyWidget);

    // Create the resources widget and add it to the hierarchy dock
    resourcesWidget = new ResourcesWidget();
    uiMainWindow->resourcesDock->setWidget(resourcesWidget);
    tabifyDockWidget(uiMainWindow->hierarchyDock, uiMainWindow->resourcesDock);
    uiMainWindow->hierarchyDock->raise();

    // Create the inspector widget and add it to the inspector dock
    inspectorWidget = new InspectorWidget();
    uiMainWindow->inspectorDock->setWidget(inspectorWidget);

    //tabifyDockWidget(uiMainWindow->hierarchyDock, uiMainWindow->inspectorDock);

    // View menu actions
    createPanelVisibilityAction(uiMainWindow->hierarchyDock);
    createPanelVisibilityAction(uiMainWindow->resourcesDock);
    createPanelVisibilityAction(uiMainWindow->inspectorDock);

    // Signals / slots connections
    connect(uiMainWindow->actionOpenProject, SIGNAL(triggered()), this, SLOT(openProject()));
    connect(uiMainWindow->actionSaveProject, SIGNAL(triggered()), this, SLOT(saveProject()));
    connect(uiMainWindow->actionSaveScreenshot, SIGNAL(triggered()), this, SLOT(saveScreenshot()));
    connect(uiMainWindow->actionAboutOpenGL, SIGNAL(triggered()), this, SLOT(showAboutOpenGL()));
    connect(uiMainWindow->actionExit, SIGNAL(triggered()), this, SLOT(exit()));
    connect(uiMainWindow->actionAddCube, SIGNAL(triggered()), this, SLOT(addCube()));
    connect(uiMainWindow->actionAddPlane, SIGNAL(triggered()), this, SLOT(addPlane()));
    connect(uiMainWindow->actionAddSphere, SIGNAL(triggered()), this, SLOT(addSphere()));
    connect(uiMainWindow->actionAddMesh, SIGNAL(triggered()), this, SLOT(addMesh()));
    connect(uiMainWindow->actionAddTexture, SIGNAL(triggered()), this, SLOT(addTexture()));
    connect(uiMainWindow->actionAddMaterial, SIGNAL(triggered()), this, SLOT(addMaterial()));

    connect(hierarchyWidget, SIGNAL(entityAdded(Entity *)), this, SLOT(onEntityAdded(Entity *)));
    connect(hierarchyWidget, SIGNAL(entityRemoved(Entity *)), this, SLOT(onEntityRemoved(Entity *)));
    connect(hierarchyWidget, SIGNAL(entitySelected(Entity *)), this, SLOT(onEntitySelected(Entity *)));
    connect(resourcesWidget, SIGNAL(resourceAdded(Resource *)), this, SLOT(onResourceAdded(Resource *)));
    connect(resourcesWidget, SIGNAL(resourceRemoved(Resource *)), this, SLOT(onResourceRemoved(Resource *)));
    connect(resourcesWidget, SIGNAL(resourceSelected(Resource *)), this, SLOT(onResourceSelected(Resource *)));
    connect(inspectorWidget, SIGNAL(entityChanged(Entity*)), this, SLOT(onEntityChanged(Entity*)));
    connect(inspectorWidget, SIGNAL(resourceChanged(Resource*)), this, SLOT(onResourceChanged(Resource*)));

    hierarchyWidget->updateLayout();
    resourcesWidget->updateLayout();
}

MainWindow::~MainWindow()
{
    delete uiMainWindow;

    // In globals.h / globals.cpp
    delete scene;

    uiMainWindow->openGLWidget->makeCurrent();
    delete resourceManager;

    g_MainWindow = nullptr;
}

void MainWindow::openProject()
{
    QString path = QFileDialog::getOpenFileName(this,"Open project");
    if (!path.isEmpty())
    {
        QFile openFile(path);

        if (!openFile.open(QIODevice::ReadOnly)) {
            qWarning("Couldn't open save file.");
            return;
        }

        QJsonDocument openDoc = QJsonDocument::fromJson(openFile.readAll());
        scene->read(openDoc.object());
        resourceManager->read(openDoc.object());

        updateEverything();
    }
}

void MainWindow::saveProject()
{
    QString path = QFileDialog::getSaveFileName(this, "Save project");
    if (!path.isEmpty())
    {
        QFile saveFile(path);

        if (!saveFile.open(QIODevice::WriteOnly)) {
            qWarning("Couldn't open save file.");
            return;
        }

        QJsonObject project;
        scene->write(project);
        QJsonDocument saveDoc(project);
        saveFile.write(saveDoc.toJson());
    }
}

void MainWindow::saveScreenshot()
{
    QString path = QFileDialog::getSaveFileName(this, "Save screenshot", QString(), "*.png");
    if (!path.isEmpty()) {
        QImage image = uiMainWindow->openGLWidget->getScreenshot();
        image.save(path);
    }
}

void MainWindow::showAboutOpenGL()
{
    AboutOpenGLDialog dialog;
    dialog.setContents(uiMainWindow->openGLWidget->getOpenGLInfo());
    dialog.exec();
}

void MainWindow::addCube()
{
    Entity *entity = scene->addEntity();
    entity->name = "Cube";
    entity->addMeshRendererComponent();
    entity->meshRenderer->mesh = resourceManager->cube;
    onEntityAdded(entity);
}

void MainWindow::addPlane()
{
    Entity *entity = scene->addEntity();
    entity->name = "Plane";
    entity->addMeshRendererComponent();
    entity->meshRenderer->mesh = resourceManager->plane;
    onEntityAdded(entity);
}

void MainWindow::addSphere()
{
    Entity *entity = scene->addEntity();
    entity->name = "Sphere";
    entity->addMeshRendererComponent();
    entity->meshRenderer->mesh = resourceManager->sphere;
    onEntityAdded(entity);
}

void MainWindow::addMesh()
{
    Mesh *res = resourceManager->createMesh();
    res->name = "Mesh";
    onResourceAdded(res);
}

void MainWindow::addTexture()
{
    Texture *res = resourceManager->createTexture();
    res->name = "Texture";
    onResourceAdded(res);
}

void MainWindow::addMaterial()
{
    Material *res = resourceManager->createMaterial();
    res->name = "Material";
    onResourceAdded(res);
}

void MainWindow::exit()
{
    QMessageBox::StandardButton button = QMessageBox::question(
                this,
                "Exit application",
                "Are you sure you want to exit the application?");
    if (button == QMessageBox::Yes) {
        qApp->quit();
    }
}

void MainWindow::updateEverything()
{
    hierarchyWidget->updateLayout();
    uiMainWindow->openGLWidget->update();
}

void MainWindow::onEntityAdded(Entity * entity)
{
    updateEverything();
    inspectorWidget->showEntity(entity);
}

void MainWindow::onEntityRemoved(Entity * /*entity*/)
{
    inspectorWidget->showEntity(nullptr);
    uiMainWindow->openGLWidget->update();
}

void MainWindow::onEntitySelected(Entity *entity)
{
    inspectorWidget->showEntity(entity);
}

void MainWindow::onEntityChanged(Entity * /*entity*/)
{
   hierarchyWidget->updateLayout();
   uiMainWindow->openGLWidget->update();
}

void MainWindow::onResourceAdded(Resource *resource)
{
    resourcesWidget->updateLayout();
    inspectorWidget->showResource(resource);
}

void MainWindow::onResourceRemoved(Resource *resource)
{
    resourcesWidget->updateLayout();
    inspectorWidget->showResource(resource);
    uiMainWindow->openGLWidget->update();
}

void MainWindow::onResourceSelected(Resource *resource)
{
    inspectorWidget->showResource(resource);
}

void MainWindow::onResourceChanged(Resource *resource)
{
    resourcesWidget->updateLayout();
    uiMainWindow->openGLWidget->update();
}

void MainWindow::createPanelVisibilityAction(QDockWidget *widget)
{
    auto action = new QAction(widget->windowTitle(), this);
    action->setCheckable(true);
    action->setChecked(true);
    connect(action, SIGNAL(triggered(bool)), widget, SLOT(setVisible(bool)));
    connect(widget, SIGNAL(visibilityChanged(bool)), action, SLOT(setChecked(bool)));
    uiMainWindow->menuView->addAction(action);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button = QMessageBox::question(
                this,
                "Exit application",
                "Are you sure you want to exit the application?");
    if (button == QMessageBox::Yes) {
        event->accept();
    } else {
        event->ignore();
    }
}
