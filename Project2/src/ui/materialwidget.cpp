#include "materialwidget.h"
#include "ui_materialwidget.h"
#include "resources/material.h"
#include <QColorDialog>


MaterialWidget::MaterialWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MaterialWidget)
{
    ui->setupUi(this);

    ui->buttonAlbedo->setText("");
    connect(ui->buttonAlbedo, SIGNAL(clicked()), this, SLOT(onButtonAlbedoChanged()));
    connect(ui->sliderSmoothness, SIGNAL(valueChanged(int)), this, SLOT(onSmoothnessChanged(int)));
}

MaterialWidget::~MaterialWidget()
{
    delete ui;
}


void MaterialWidget::setMaterial(Material *m)
{
    material = m;

    if (material != nullptr)
    {
        ui->buttonAlbedo->setStyleSheet(
                    QString::fromLatin1("background-color: rgb(%0, %1, %2);")
                    .arg(material->albedo.red())
                    .arg(material->albedo.green())
                    .arg(material->albedo.blue()));
        ui->sliderSmoothness->setValue(material->smoothness * 255);
    }
}

void MaterialWidget::onButtonAlbedoChanged()
{
    QColor color = QColorDialog::getColor(material->albedo, this, "Albedo");
    if (color.isValid())
    {
        material->albedo = color;
        ui->buttonAlbedo->setStyleSheet(
                    QString::fromLatin1("background-color: rgb(%0, %1, %2);")
                    .arg(material->albedo.red())
                    .arg(material->albedo.green())
                    .arg(material->albedo.blue()));
        emit resourceChanged(material);
    }
}

void MaterialWidget::onSmoothnessChanged(int value)
{
    material->smoothness = value / 255.0f;//ui->sliderSmoothness->maximumValue();
    emit resourceChanged(material);
}
