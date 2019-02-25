#include "ui/inspectorwidget.h"
#include "ui/entitywidget.h"
#include "ui/transformwidget.h"
#include "ui/meshrendererwidget.h"
#include "ui/componentwidget.h"
#include "ui/mainwindow.h"
#include "ecs/scene.h"
#include <QLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QPushButton>

InspectorWidget::InspectorWidget(QWidget *parent) :
    QWidget(parent)
{
    // Create subwidgets independently
    transformWidget = new TransformWidget;
    meshRendererWidget = new MeshRendererWidget;
    QSpacerItem *spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Add all elements to the layout
    entityWidget = new EntityWidget;

    transformComponentWidget = new ComponentWidget;
    transformComponentWidget->setWidget(transformWidget);

    meshRendererComponentWidget = new ComponentWidget;
    meshRendererComponentWidget->setWidget(meshRendererWidget);

    buttonAddMeshRenderer = new QPushButton("Add Mesh Renderer");

    // Create a vertical layout for this widget
    layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(entityWidget);
    layout->addWidget(transformComponentWidget);
    layout->addWidget(meshRendererComponentWidget);
    layout->addWidget(buttonAddMeshRenderer);
    layout->addItem(spacer);

    // Set the layout for this widget
    setLayout(layout);

    showEntity(nullptr);

    connect(entityWidget, SIGNAL(entityChanged(Entity*)), this, SLOT(onEntityChanged(Entity *)));
    connect(transformWidget, SIGNAL(componentChanged(Component*)), this, SLOT(onComponentChanged(Component *)));
    connect(meshRendererWidget, SIGNAL(componentChanged(Component*)), this, SLOT(onComponentChanged(Component *)));
    connect(entityWidget, SIGNAL(entityChanged(Entity*)), this, SLOT(onEntityChanged(Entity *)));
    connect(buttonAddMeshRenderer, SIGNAL(clicked()), this, SLOT(onAddMeshRendererClicked()));
    connect(meshRendererComponentWidget, SIGNAL(removeClicked(Component*)), this, SLOT(onRemoveComponent(Component *)));
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

void InspectorWidget::onAddMeshRendererClicked()
{
    if (entity == nullptr) return;
    entity->addMeshRendererComponent();
    updateLayout();
    emit entityChanged(entity);
}

void InspectorWidget::onRemoveComponent(Component *c)
{
    if (entity == nullptr) return;
    entity->removeComponent(c);
    updateLayout();
    emit entityChanged(entity);
}

void InspectorWidget::updateLayout()
{
    entityWidget->setVisible(entity != nullptr);
    transformComponentWidget->setVisible(entity != nullptr && entity->transform != nullptr);
    meshRendererComponentWidget->setVisible(entity != nullptr && entity->meshRenderer != nullptr);
    buttonAddMeshRenderer->setVisible(entity != nullptr && entity->meshRenderer == nullptr);

    if (entity == nullptr) return;

    transformComponentWidget->setComponent(entity->transform);
    meshRendererComponentWidget->setComponent(entity->meshRenderer);

    entityWidget->setEntity(entity);
    transformWidget->setTransform(entity->transform);
    meshRendererWidget->setMeshRenderer(entity->meshRenderer);
}
