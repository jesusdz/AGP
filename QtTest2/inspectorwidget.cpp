#include "inspectorwidget.h"
#include "transformwidget.h"
#include "meshrendererwidget.h"
#include "componentwidget.h"
#include <QLayout>
#include <QVBoxLayout>
#include <QSpacerItem>

InspectorWidget::InspectorWidget(QWidget *parent) :
    QWidget(parent)
{
    // Create subwidgets independently
    TransformWidget *transformWidget = new TransformWidget;
    MeshRendererWidget *meshRendererWidget = new MeshRendererWidget;
    QSpacerItem *spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Create a vertical layout for this widget
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);

    // Add all elements to the layout
    auto componentWidget = new ComponentWidget;
    componentWidget->setWidget(transformWidget);
    layout->addWidget(componentWidget);

    componentWidget = new ComponentWidget;
    componentWidget->setWidget(meshRendererWidget);
    layout->addWidget(componentWidget);

    layout->addItem(spacer);

    // Set the layout for this widget
    setLayout(layout);
}

InspectorWidget::~InspectorWidget()
{
}
