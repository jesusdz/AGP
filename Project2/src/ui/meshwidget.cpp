#include "meshwidget.h"
#include "ui_meshwidget.h"
#include "resources/mesh.h"
#include "resources/resourcemanager.h"
#include "globals.h"

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

    mesh->loadModel(":/models/Patrick.obj");

    emit resourceChanged(mesh);
}
