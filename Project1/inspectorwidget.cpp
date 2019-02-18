#include "inspectorwidget.h"
#include "entitywidget.h"
#include "transformwidget.h"
#include "shaperendererwidget.h"
#include "backgroundrendererwidget.h"
#include "componentwidget.h"
#include "Scene.h"
#include "mainwindow.h"
#include <QLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QPushButton>

InspectorWidget::InspectorWidget(QWidget *parent) :
    QWidget(parent)
{
    // Create subwidgets independently
    transformWidget = new TransformWidget;
    shapeRendererWidget = new ShapeRendererWidget;
    backgroundRendererWidget = new BackgroundRendererWidget;
    QSpacerItem *spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Add all elements to the layout
    entityWidget = new EntityWidget;

    transformComponentWidget = new ComponentWidget;
    transformComponentWidget->setWidget(transformWidget);

    shapeRendererComponentWidget = new ComponentWidget;
    shapeRendererComponentWidget->setWidget(shapeRendererWidget);

    backgroundRendererComponentWidget = new ComponentWidget;
    backgroundRendererComponentWidget->setWidget(backgroundRendererWidget);

    buttonAddShapeRenderer = new QPushButton("Add Shape Renderer");
    buttonAddBackgroundRenderer = new QPushButton("Add Background Renderer");

    // Create a vertical layout for this widget
    layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(entityWidget);
    layout->addWidget(transformComponentWidget);
    layout->addWidget(shapeRendererComponentWidget);
    layout->addWidget(backgroundRendererComponentWidget);
    layout->addWidget(buttonAddShapeRenderer);
    layout->addWidget(buttonAddBackgroundRenderer);
    layout->addItem(spacer);

    // Set the layout for this widget
    setLayout(layout);

    showEntity(nullptr);

    connect(entityWidget, SIGNAL(entityChanged(Entity*)), this, SLOT(onEntityChanged(Entity *)));
    connect(transformWidget, SIGNAL(componentChanged(Component*)), this, SLOT(onComponentChanged(Component *)));
    connect(shapeRendererWidget, SIGNAL(componentChanged(Component*)), this, SLOT(onComponentChanged(Component *)));
    connect(backgroundRendererWidget, SIGNAL(componentChanged(Component*)), this, SLOT(onComponentChanged(Component *)));
    connect(entityWidget, SIGNAL(entityChanged(Entity*)), this, SLOT(onEntityChanged(Entity *)));
    connect(buttonAddShapeRenderer, SIGNAL(clicked()), this, SLOT(onAddShapeRendererClicked()));
    connect(buttonAddBackgroundRenderer, SIGNAL(clicked()), this, SLOT(onAddBackgroundRendererClicked()));
}

InspectorWidget::~InspectorWidget()
{
}

void InspectorWidget::showEntity(Entity *e)
{
    entity = e;
    updateLayout();
}

void InspectorWidget::onEntityChanged(Entity *entity)
{
    emit entityChanged(entity);
}

void InspectorWidget::onComponentChanged(Component *)
{
    emit entityChanged(entity);
}

void InspectorWidget::onAddShapeRendererClicked()
{
    if (entity == nullptr) return;
    entity->addShapeRendererComponent();
    updateLayout();
}

void InspectorWidget::onAddBackgroundRendererClicked()
{
    if (entity == nullptr) return;
    entity->addBackgroundRendererComponent();
    updateLayout();
}

void InspectorWidget::updateLayout()
{
    entityWidget->setVisible(entity != nullptr);
    transformComponentWidget->setVisible(entity != nullptr && entity->transform != nullptr);
    shapeRendererComponentWidget->setVisible(entity != nullptr && entity->shapeRenderer != nullptr);
    backgroundRendererComponentWidget->setVisible(entity != nullptr && entity->backgroundRenderer != nullptr);
    buttonAddShapeRenderer->setVisible(entity != nullptr && entity->shapeRenderer == nullptr);
    buttonAddBackgroundRenderer->setVisible(entity != nullptr && entity->backgroundRenderer == nullptr);

    if (entity == nullptr) return;

    entityWidget->setEntity(entity);
    transformWidget->setTransform(entity->transform);
    shapeRendererWidget->setShapeRenderer(entity->shapeRenderer);
    backgroundRendererWidget->setBackgroundRenderer(entity->backgroundRenderer);
}
