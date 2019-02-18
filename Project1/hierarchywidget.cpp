#include "hierarchywidget.h"
#include "ui_hierarchywidget.h"
#include "mainwindow.h"
#include "inspectorwidget.h"
#include "scene.h"
#include <iostream>

HierarchyWidget::HierarchyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HierarchyWidget)
{
    ui->setupUi(this);

    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addEntity()));
    connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(removeEntity()));
    connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));
}

HierarchyWidget::~HierarchyWidget()
{
    delete ui;
}

void HierarchyWidget::updateEntityList()
{
    ui->listWidget->clear();
    for (int i = 0; i < g_Scene->numEntities(); ++i)
    {
        if (g_Scene->entityAt(i) != nullptr)
        {
            ui->listWidget->addItem(g_Scene->entityAt(i)->name);
        }
    }
}

void HierarchyWidget::addEntity()
{
    g_Scene->addEntity();
    updateEntityList();
}

void HierarchyWidget::removeEntity()
{
    int index = ui->listWidget->currentRow();
    if (index != -1)
    {
        ui->listWidget->takeItem(index);
        g_Scene->removeEntityAt(index);
    }
}

void HierarchyWidget::onItemSelectionChanged()
{
    int index = ui->listWidget->currentRow();
    if (index != -1)
    {
        Entity *entity = g_Scene->entityAt(index);
        g_MainWindow->inspectorWidget->showEntity(entity);
    }
}
