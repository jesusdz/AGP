#include "ui/meshrendererwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include "ecs/scene.h"
#include "resources/mesh.h"
#include "resources/resourcemanager.h"
#include "globals.h"

MeshRendererWidget::MeshRendererWidget(QWidget *parent) :
    QWidget(parent)
{
    QFont font;
    font.setBold(true);

    setWindowTitle(QString::fromLatin1("Mesh Renderer"));

    auto vlayout = new QVBoxLayout;

    { // Shape
        auto label = new QLabel;
        label->setText("Shape");

        comboMesh = new QComboBox;
        comboMesh->addItem("Mesh 1");
        comboMesh->addItem("Mesh 2");
        comboMesh->addItem("Mesh 3");

        auto hlayout = new QHBoxLayout;
        hlayout->addWidget(label);
        hlayout->addWidget(comboMesh);
        vlayout->addItem(hlayout);
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

    comboMesh->clear();

    comboMesh->addItem("None", QVariant::fromValue<void*>(nullptr));

    for (auto mesh : resourceManager->meshes)
    {
        comboMesh->addItem(mesh->name, QVariant::fromValue<void*>(mesh));
    }
}

void MeshRendererWidget::onMeshChanged(int index)
{
    meshRenderer->mesh = (Mesh*) comboMesh->itemData(index).value<void*>();
    emit componentChanged(meshRenderer);
}
