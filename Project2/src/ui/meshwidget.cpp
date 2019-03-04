#include "meshwidget.h"
#include "ui_meshwidget.h"
#include "resources/mesh.h"
#include "resources/resourcemanager.h"
#include "globals.h"
#include <QFileDialog>


MeshWidget::MeshWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MeshWidget)
{
    ui->setupUi(this);
    connect(ui->buttonOpen, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
}

MeshWidget::~MeshWidget()
{
    delete ui;
}

void MeshWidget::setMesh(Mesh *m)
{
    mesh = m;
}

void MeshWidget::onButtonClicked()
{
    if (mesh == nullptr) return;

    QString path = QFileDialog::getOpenFileName(this,"Load model file", QString(), QString::fromLatin1("3D meshes (*.obj *.fbx)"));
    if (!path.isEmpty())
    {
        mesh->loadModel(path.toLatin1());
        emit resourceChanged(mesh);
    }
}
