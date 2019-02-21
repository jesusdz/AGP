#include "transformwidget.h"
#include "ui_transform.h"

Transform::Transform(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Transform)
{
    ui->setupUi(this);
}

Transform::~Transform()
{
    delete ui;
}
