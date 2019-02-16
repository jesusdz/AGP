#include "inspectorwidget.h"
#include "transformwidget.h"
#include "meshrendererwidget.h"
#include <QLayout>
#include <QVBoxLayout>
#include <QSpacerItem>

InspectorWidget::InspectorWidget(QWidget *parent) :
    QWidget(parent)
{
    TransformWidget *transformWidget = new TransformWidget;
    MeshRendererWidget *meshRendererWidget = new MeshRendererWidget;
    QSpacerItem *spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);


    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(transformWidget);
    layout->addWidget(meshRendererWidget);
    layout->addItem(spacer);
    setLayout(layout);
}

InspectorWidget::~InspectorWidget()
{
}
