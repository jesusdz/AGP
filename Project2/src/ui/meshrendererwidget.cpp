#include "ui/meshrendererwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include "ecs/scene.h"
#include "resources/mesh.h"
#include "resources/resourcemanager.h"
#include "globals.h"
#include <QPushButton>
#include <QListWidget>


MeshRendererWidget::MeshRendererWidget(QWidget *parent) :
    QWidget(parent)
{
    QFont font;
    font.setBold(true);

    setWindowTitle(QString::fromLatin1("Mesh Renderer"));

    auto vlayout = new QVBoxLayout;

    { // Mesh
        auto label = new QLabel;
        label->setText("Mesh");

        comboMesh = new QComboBox;

        auto hlayout = new QHBoxLayout;
        hlayout->addWidget(label);
        hlayout->addWidget(comboMesh);
        vlayout->addItem(hlayout);
    }

    { // List of materials
        auto label = new QLabel;
        label->setText("Materials");

        auto addButton = new QPushButton("Add");
        connect(addButton, SIGNAL(clicked()), this, SLOT(addMaterial()));

        auto hlayout = new QHBoxLayout;
        hlayout->addWidget(label);
        hlayout->addWidget(addButton);
        vlayout->addItem(hlayout);

        // List
        materialsList = new QListWidget;
        vlayout->addWidget(materialsList);
    }

    setLayout(vlayout);

    connect(comboMesh, SIGNAL(currentIndexChanged(int)), this, SLOT(onMeshChanged(int)));
}

MeshRendererWidget::~MeshRendererWidget()
{
}

void MeshRendererWidget::setMeshRenderer(MeshRenderer *m)
{
    meshRenderer = m;
    if (meshRenderer == nullptr) return;

    bool wasBlocked = comboMesh->blockSignals(true);

    comboMesh->clear();

    comboMesh->addItem("None", QVariant::fromValue<void*>(nullptr));

    for (auto resource : resourceManager->resources)
    {
        Mesh *mesh = resource->asMesh();

        if (mesh != nullptr)
        {
            comboMesh->addItem(mesh->name, QVariant::fromValue<void*>(mesh));

            if (m->mesh == mesh)
            {
                comboMesh->setCurrentIndex(comboMesh->count() - 1);
            }
        }
    }

    comboMesh->blockSignals(wasBlocked);
}

void MeshRendererWidget::onMeshChanged(int index)
{
    meshRenderer->mesh = (Mesh*) comboMesh->itemData(index).value<void*>();
    emit componentChanged(meshRenderer);
}

void MeshRendererWidget::addMaterial()
{
    // TODO: meshRenderer->addMaterial()
    materialsList->addItem("Material");
}
