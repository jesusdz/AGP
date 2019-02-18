#include "hierarchywidget.h"
#include "ui_hierarchywidget.h"

HierarchyWidget::HierarchyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HierarchyWidget)
{
    ui->setupUi(this);

    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addEntity()));
    connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(removeEntity()));
}

HierarchyWidget::~HierarchyWidget()
{
    delete ui;
}

void HierarchyWidget::addEntity()
{

}

void HierarchyWidget::removeEntity()
{

}
