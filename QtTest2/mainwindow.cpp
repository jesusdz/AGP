#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hierarchywidget.h"
#include "inspectorwidget.h"
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    uiMainWindow(new Ui::MainWindow)
{
    uiMainWindow->setupUi(this);

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
}

MainWindow::~MainWindow()
{
    delete uiMainWindow;
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
