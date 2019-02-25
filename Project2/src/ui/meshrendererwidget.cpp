#include "meshrendererwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include "../scene.h"

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

    // TODO Loop to look for names
    //comboMesh->setCurrentIndex();
}

void MeshRendererWidget::onMeshChanged(int index)
{
    //meshRenderer->mesh = nullptr; // TODO: find the mesh and set it
    emit componentChanged(meshRenderer);
}
