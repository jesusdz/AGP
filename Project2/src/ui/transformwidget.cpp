#include "ui/transformwidget.h"
#include "ui_transformwidget.h"
#include "ecs/scene.h"
#include <QSignalBlocker>

TransformWidget::TransformWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransformWidget)
{
    ui->setupUi(this);
    ui->spinTx->setRange(0, 1000);
    ui->spinTy->setRange(0, 1000);
    ui->spinSx->setRange(0.001, 1000);
    ui->spinSy->setRange(0.001, 1000);

    connect(ui->spinTx, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
    connect(ui->spinTy, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
    connect(ui->spinSx, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
    connect(ui->spinSy, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
}

TransformWidget::~TransformWidget()
{
    delete ui;
}

void TransformWidget::setTransform(Transform *t)
{
    transform = t;
    if (transform == nullptr) return;

    QSignalBlocker txb(ui->spinTx);
    QSignalBlocker tyb(ui->spinTy);
    QSignalBlocker sxb(ui->spinSx);
    QSignalBlocker syb(ui->spinSy);
    ui->spinTx->setValue(t->tx);
    ui->spinTy->setValue(t->ty);
    ui->spinSx->setValue(t->sx);
    ui->spinSy->setValue(t->sy);
}

void TransformWidget::onValueChanged(double)
{
    transform->tx = ui->spinTx->value();
    transform->ty = ui->spinTy->value();
    transform->sx = ui->spinSx->value();
    transform->sy = ui->spinSy->value();
    emit componentChanged(transform);
}
