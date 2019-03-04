#include "ui/inspectorwidget.h"
#include "ui/entitywidget.h"
#include "ui/transformwidget.h"
#include "ui/meshrendererwidget.h"
#include "ui/componentwidget.h"
#include "ui/mainwindow.h"
#include "ui/meshwidget.h"
#include "ui/texturewidget.h"
#include "ui/resourcewidget.h"
#include "ecs/scene.h"
#include "resources/resource.h"
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

    resourceWidget = new ResourceWidget;

    meshWidget = new MeshWidget;
    textureWidget = new TextureWidget;

    // Create a vertical layout for this widget
    layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(entityWidget);
    layout->addWidget(transformComponentWidget);
    layout->addWidget(meshRendererComponentWidget);
    layout->addWidget(buttonAddMeshRenderer);
    layout->addWidget(resourceWidget);
    layout->addWidget(meshWidget);
    layout->addWidget(textureWidget);
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

    connect(resourceWidget, SIGNAL(resourceChanged(Resource*)), this, SLOT(onResourceChanged(Resource *)));
    connect(meshWidget, SIGNAL(resourceChanged(Resource*)), this, SLOT(onResourceChanged(Resource*)));
    connect(textureWidget, SIGNAL(resourceChanged(Resource*)), this, SLOT(onResourceChanged(Resource*)));

    updateLayout();
}

InspectorWidget::~InspectorWidget()
{
}

void InspectorWidget::showEntity(Entity *e)
{
    entity = e;
    resource = nullptr;
    updateLayout();
}

void InspectorWidget::showResource(Resource *r)
{
    entity = nullptr;
    resource = r;
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

void InspectorWidget::onResourceChanged(Resource *res)
{
    updateLayout();
    emit resourceChanged(res);
}

void InspectorWidget::updateLayout()
{
    entityWidget->setVisible(false);
    transformComponentWidget->setVisible(false);
    meshRendererComponentWidget->setVisible(false);
    buttonAddMeshRenderer->setVisible(false);
    resourceWidget->setVisible(false);
    meshWidget->setVisible(false);
    textureWidget->setVisible(false);

    // Entity related
    if (entity != nullptr)
    {
        entityWidget->setVisible(true);
        transformComponentWidget->setVisible(entity->transform != nullptr);
        meshRendererComponentWidget->setVisible(entity->meshRenderer != nullptr);
        buttonAddMeshRenderer->setVisible(entity->meshRenderer == nullptr);

        transformComponentWidget->setComponent(entity->transform);
        meshRendererComponentWidget->setComponent(entity->meshRenderer);

        entityWidget->setEntity(entity);
        transformWidget->setTransform(entity->transform);
        meshRendererWidget->setMeshRenderer(entity->meshRenderer);
    }

    // Resource related
    if (resource != nullptr)
    {
        Mesh *mesh = resource->asMesh();
        Texture *texture = resource->asTexture();

        resourceWidget->setVisible(true);
        meshWidget->setVisible(mesh != nullptr);
        textureWidget->setVisible(texture != nullptr);

        resourceWidget->setResource(resource);
        meshWidget->setMesh(mesh);
        textureWidget->setTexture(texture);
    }
}
