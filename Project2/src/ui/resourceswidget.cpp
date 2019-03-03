#include "ui/resourceswidget.h"
#include "ui_resourceswidget.h"
#include "resources/resource.h"
#include "resources/resourcemanager.h"
#include "resources/texture.h"
#include "resources/mesh.h"
#include "resources/material.h"
#include "globals.h"
#include <QFileDialog>


ResourcesWidget::ResourcesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResourcesWidget)
{
    ui->setupUi(this);

    connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(removeResource()));
    connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));
}

ResourcesWidget::~ResourcesWidget()
{
    delete ui;
}

void ResourcesWidget::updateLayout()
{
    ui->listWidget->clear();
    for (int i = 0; i < resourceManager->numResources(); ++i)
    {
        Resource *res = resourceManager->resourceAt(i);
        if (res != nullptr)
        {
            QString type = QString::fromLatin1("unknown");
            if (res->asMesh() != nullptr) type = QString::fromLatin1("mesh");
            if (res->asTexture() != nullptr) type = QString::fromLatin1("texture");
            if (res->asMaterial() != nullptr) type = QString::fromLatin1("material");

            ui->listWidget->addItem(QString::fromLatin1("%0 (%1)").arg(res->name).arg(type));
        }
    }
}

void ResourcesWidget::addResource()
{
    QString path = QFileDialog::getOpenFileName(this, "Import a resource", QString(), "PNG (*.png *.jpg);;JPEG (*.jpg *.jpeg)");
    if (!path.isEmpty())
    {
        Resource *res = nullptr;
        if (path.endsWith(".png") || path.endsWith(".jpg"))
        {
            Texture *tex = resourceManager->createTexture();
            tex->name = path;
            tex->loadTexture(path.toLatin1());
            res = tex;
        }
        if (path.endsWith(".obj") || path.endsWith(".fbx"))
        {
            Mesh *mesh = resourceManager->createMesh();
            mesh->name = path;
            mesh->loadModel(path.toLatin1());
            res = mesh;
        }
        updateLayout();
        emit resourceAdded(res);
    }
}

void ResourcesWidget::removeResource()
{
    int index = ui->listWidget->currentRow();
    if (index != -1)
    {
        ui->listWidget->takeItem(index);
        resourceManager->removeResourceAt(index);
    }
    emit resourceRemoved(nullptr);
}

void ResourcesWidget::onItemSelectionChanged()
{
    int index = ui->listWidget->currentRow();
    if (index != -1)
    {
        Resource *res = resourceManager->resourceAt(index);
        emit resourceSelected(res);
    }
}
