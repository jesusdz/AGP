#include "ui/meshrendererwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include "ecs/scene.h"
#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/resourcemanager.h"
#include "globals.h"
#include <QPushButton>
#include <QListView>


MeshRendererWidget::MeshRendererWidget(QWidget *parent) :
    QWidget(parent)
{
    QFont font;
    font.setBold(true);

    setWindowTitle(QString::fromLatin1("Mesh Renderer"));

    updateLayout();
}

MeshRendererWidget::~MeshRendererWidget()
{
}

void MeshRendererWidget::setMeshRenderer(MeshRenderer *m)
{
    meshRenderer = m;
    if (meshRenderer == nullptr) return;
}

void MeshRendererWidget::onMeshChanged(int index)
{
    meshRenderer->mesh = (Mesh*) comboMesh->itemData(index).value<void*>();
    emit componentChanged(meshRenderer);
}

void MeshRendererWidget::onMaterialChanged(int comboIndex)
{
    QComboBox *combo = (QComboBox*)sender();
    for (int i = 0; i < combosMaterial.size(); ++i)
    {
        if (combo == combosMaterial[i])
        {
            Material *material = nullptr;
            if (comboIndex > 0) {
                material = (Material*)combo->itemData(comboIndex).value<void*>();
            }
            meshRenderer->materials[i] = material;
            emit componentChanged(meshRenderer);
        }
    }
}

void MeshRendererWidget::addMaterial()
{
    if (meshRenderer == nullptr) {
        qDebug("No hay meshRenderer.");
        return;
    }
    meshRenderer->materials.push_back(nullptr);
    updateLayout();
    emit componentChanged(meshRenderer);
}

void MeshRendererWidget::destroyLayout()
{
    hide();
    QVector<QLayoutItem*> items;
    items.push_back(layout());

    while (!items.empty())
    {
        QLayoutItem *item = items.takeAt(0);
        if (item != nullptr)
        {
            if (item->layout())
            {
                QLayoutItem *subitem;
                while ((subitem = item->layout()->takeAt(0)) != nullptr)
                {
                    items.push_back(subitem);
                }
                delete item->layout();
            }
            else if (item->widget() != nullptr)
            {
                item->widget()->hide();
                delete item->widget();
            }
            else
            {
                delete item;
            }
        }
    }

    combosMaterial.clear();
}

void MeshRendererWidget::updateLayout()
{
    destroyLayout();

    auto vlayout = new QVBoxLayout;

    { // Mesh
        auto label = new QLabel;
        label->setText("Mesh");

        comboMesh = createMeshesCombo();

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
        materialsList = createMaterialsLayout();
        vlayout->addItem(materialsList);
    }

    setLayout(vlayout);
    show();

    connect(comboMesh, SIGNAL(currentIndexChanged(int)), this, SLOT(onMeshChanged(int)));
}

QComboBox *MeshRendererWidget::createMeshesCombo()
{
    auto comboMesh = new QComboBox;

    bool wasBlocked = comboMesh->blockSignals(true);

    comboMesh->clear();

    comboMesh->addItem("None", QVariant::fromValue<void*>(nullptr));

    for (auto resource : resourceManager->resources)
    {
        Mesh *mesh = resource->asMesh();

        if (mesh != nullptr)
        {
            comboMesh->addItem(mesh->name, QVariant::fromValue<void*>(mesh));

            if (meshRenderer != nullptr && meshRenderer->mesh == mesh)
            {
                comboMesh->setCurrentIndex(comboMesh->count() - 1);
            }
        }
    }

    comboMesh->blockSignals(wasBlocked);

    return comboMesh;
}
QVBoxLayout *MeshRendererWidget::createMaterialsLayout()
{
    auto listLayout = new QVBoxLayout;
    listLayout->setSpacing(0);
    listLayout->setMargin(0);

    if (meshRenderer == nullptr) {
        return listLayout;
    }

    for (int i = 0; i < meshRenderer->materials.size(); ++i)
    {
        Material *material = meshRenderer->materials[i];
        auto layout = new QHBoxLayout;
        auto label = new QLabel(QString::fromLatin1("[%0]").arg(i));
        auto combo = createComboForMaterial(material);
        layout->addWidget(label);
        layout->addWidget(combo);

        listLayout->addItem(layout);
    }

    return listLayout;
}

QComboBox * MeshRendererWidget::createComboForMaterial(Material *material)
{
    auto combo = new QComboBox;
    combo->addItem("None");
    int selectedIndex = 0;
    combosMaterial.push_back(combo);

    int i = 1;
    for (auto res : resourceManager->resources)
    {
        Material *mat = res->asMaterial();
        if (mat != nullptr)
        {
            if (mat == material) {
                selectedIndex = i;
            }
            combo->addItem(mat->name, QVariant::fromValue<void*>(mat));
            combo->userData(0);
            connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(onMaterialChanged(int)));
            ++i;
        }
    }

    combo->setCurrentIndex(selectedIndex);
    return combo;
}
