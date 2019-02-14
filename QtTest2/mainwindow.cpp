#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_rendering.h"
#include "inspector.h"
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    uiMainWindow(new Ui::MainWindow),
    uiRendering(new Ui::Rendering)
{
    uiMainWindow->setupUi(this);

    QDockWidget *dockWidget = new QDockWidget;
    dockWidget->setWindowTitle("Lighting");
    this->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
    tabifyDockWidget(uiMainWindow->renderingDock, dockWidget);

    // If the rendering dock was floating (specified in the designer)
    // place it in the same docking area as the inspector as a tab
    uiMainWindow->renderingDock->setFloating(false);
    tabifyDockWidget(uiMainWindow->renderingDock, uiMainWindow->inspectorDock);

    // All tab positions on top of the docking area
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::TabPosition::North);

    // Create the rendering widget...
    QWidget *renderingWidget = new QWidget();
    uiRendering->setupUi(renderingWidget);

    // ... and add it to the rendering dock
    uiMainWindow->renderingDock->setWidget(renderingWidget);

    // Create the inspector widget and add it to the inspector dock
    inspector = new Inspector();
    uiMainWindow->inspectorDock->setWidget(inspector);

    connect(uiMainWindow->actionOpenProject, SIGNAL(triggered()), this, SLOT(openProject()));
    connect(uiMainWindow->actionSaveProject, SIGNAL(triggered()), this, SLOT(saveProject()));
    connect(uiMainWindow->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
}

MainWindow::~MainWindow()
{
    delete uiMainWindow;
    delete uiRendering;
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
