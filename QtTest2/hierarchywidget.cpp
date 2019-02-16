#include "hierarchywidget.h"
#include "ui_hierarchywidget.h"

HierarchyWidget::HierarchyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HierarchyWidget)
{
    ui->setupUi(this);
}

HierarchyWidget::~HierarchyWidget()
{
    delete ui;
}
