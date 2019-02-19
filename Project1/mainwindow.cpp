#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hierarchywidget.h"
#include "inspectorwidget.h"
#include "scene.h"
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>

MainWindow *g_MainWindow = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    uiMainWindow(new Ui::MainWindow),
    scene(new Scene())
{
    g_MainWindow = this;
    uiMainWindow->setupUi(this);

    // Set the style of the window
    QFile file(":/stylesheets/stylesheet");
    if (file.open(QIODevice::ReadOnly)) {
        QString contents = file.readAll();
        setStyleSheet(contents);
    }

    // All tab positions on top of the docking area
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::TabPosition::North);

    // Create the hierarchy widget and add it to the inspector dock
    hierarchyWidget = new HierarchyWidget();
    uiMainWindow->hierarchyDock->setWidget(hierarchyWidget);

    // Create the inspector widget and add it to the inspector dock
    inspectorWidget = new InspectorWidget();
    uiMainWindow->inspectorDock->setWidget(inspectorWidget);

    //tabifyDockWidget(uiMainWindow->hierarchyDock, uiMainWindow->inspectorDock);

    // Signals / slots connections
    connect(uiMainWindow->actionOpenProject, SIGNAL(triggered()), this, SLOT(openProject()));
    connect(uiMainWindow->actionSaveProject, SIGNAL(triggered()), this, SLOT(saveProject()));
    connect(uiMainWindow->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    connect(hierarchyWidget, SIGNAL(entityAdded(Entity *)), this, SLOT(onEntityAdded(Entity *)));
    connect(hierarchyWidget, SIGNAL(entityRemoved(Entity *)), this, SLOT(onEntityRemoved(Entity *)));
    connect(hierarchyWidget, SIGNAL(entitySelected(Entity *)), this, SLOT(onEntitySelected(Entity *)));
    connect(inspectorWidget, SIGNAL(entityChanged(Entity*)), this, SLOT(onEntityChanged(Entity*)));
}

MainWindow::~MainWindow()
{
    delete uiMainWindow;
    delete scene;
    g_MainWindow = nullptr;
}

void MainWindow::openProject()
{
    QString path = QFileDialog::getOpenFileName(this,"Open project");
    if (!path.isEmpty()) {
        std::cout << path.toStdString() << std::endl;
    }
}

void MainWindow::saveProject()
{
    QMessageBox::StandardButton button = QMessageBox::question(
                this,
                "Exit application",
                "Do you want to exit the application without saving the project?");
    if (button == QMessageBox::Yes) {
        std::cout << "Exit without saving changes" << std::endl;
    } else {
        std::cout << "Cancel exit" << std::endl;
    }

    QString path = QFileDialog::getSaveFileName(this, "Save project");
    if (!path.isEmpty()) {
        std::cout << path.toStdString() << std::endl;
    }
}

void MainWindow::onEntityAdded(Entity * entity)
{
    inspectorWidget->showEntity(entity);
    uiMainWindow->myCustomWidget->update();
}

void MainWindow::onEntityRemoved(Entity * /*entity*/)
{
    inspectorWidget->showEntity(nullptr);
    uiMainWindow->myCustomWidget->update();
}

void MainWindow::onEntitySelected(Entity *entity)
{
    inspectorWidget->showEntity(entity);
}

void MainWindow::onEntityChanged(Entity * /*entity*/)
{
   hierarchyWidget->updateEntityList();
   uiMainWindow->myCustomWidget->update();
   //std::cout << "onEntityChanged" << std::endl;
}
