#include "ui/hierarchywidget.h"
#include "ui_hierarchywidget.h"
#include "ecs/scene.h"
#include "globals.h"
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
    for (int i = 0; i < scene->numEntities(); ++i)
    {
        if (scene->entityAt(i) != nullptr)
        {
            ui->listWidget->addItem(scene->entityAt(i)->name);
        }
    }
}

void HierarchyWidget::addEntity()
{
    Entity *entity = scene->addEntity();
    updateEntityList();
    emit entityAdded(entity);
}

void HierarchyWidget::removeEntity()
{
    int index = ui->listWidget->currentRow();
    if (index != -1)
    {
        ui->listWidget->takeItem(index);
        scene->removeEntityAt(index);
    }
    emit entityRemoved(nullptr);
}

void HierarchyWidget::onItemSelectionChanged()
{
    int index = ui->listWidget->currentRow();
    if (index != -1)
    {
        Entity *entity = scene->entityAt(index);
        emit entitySelected(entity);
    }
}
