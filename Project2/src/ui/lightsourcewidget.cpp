#include "lightsourcewidget.h"
#include "ecs/scene.h"
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QColorDialog>
#include <QSignalBlocker>


LightSourceWidget::LightSourceWidget(QWidget *parent) : QWidget(parent)
{
    auto vlayout = new QVBoxLayout;

    auto labelType = new QLabel("Type");
    comboType = new QComboBox;
    comboType->addItem("Point", QVariant::fromValue<int>((int)LightSource::Type::Point));
    comboType->addItem("Directional", QVariant::fromValue<int>((int)LightSource::Type::Directional));
    auto hlayout = new QHBoxLayout;
    hlayout->addWidget(labelType);
    hlayout->addWidget(comboType);
    vlayout->addItem(hlayout);

    auto labelColor = new QLabel("Color");
    buttonColor = new QPushButton("");
    hlayout = new QHBoxLayout;
    hlayout->addWidget(labelColor);
    hlayout->addWidget(buttonColor);
    vlayout->addItem(hlayout);

    auto labelIntensity = new QLabel("Intensity");
    spinIntensity = new QDoubleSpinBox();
    spinIntensity->setMinimum(0.0);
    spinIntensity->setMaximum(10.0);
    spinIntensity->setValue(1.0);
    hlayout = new QHBoxLayout;
    hlayout->addWidget(labelIntensity);
    hlayout->addWidget(spinIntensity);
    vlayout->addItem(hlayout);

    setLayout(vlayout);

    connect(comboType, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged(int)));
    connect(spinIntensity, SIGNAL(valueChanged(double)), this, SLOT(onIntensityChanged(double)));
    connect(buttonColor, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
}

void LightSourceWidget::setLightSource(LightSource *light)
{
    lightSource = light;
    if (lightSource == nullptr) return;

    QSignalBlocker b1(comboType);
    QSignalBlocker b2(spinIntensity);
    QSignalBlocker b3(buttonColor);

    comboType->setCurrentIndex((int)lightSource->type);

    spinIntensity->setValue(lightSource->intensity);

    QString colorName = lightSource->color.name();
    buttonColor->setStyleSheet(QString::fromLatin1("background-color: %0").arg(colorName));
}

void LightSourceWidget::onTypeChanged(int index)
{
    lightSource->type = (LightSource::Type)comboType->itemData(index).value<int>();
    emit componentChanged(lightSource);
}

void LightSourceWidget::onIntensityChanged(double val)
{
    lightSource->intensity = val;
    emit componentChanged(lightSource);
}

void LightSourceWidget::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(lightSource->color, this, "Light color");
    if (color.isValid())
    {
        QString colorName = color.name();
        buttonColor->setStyleSheet(QString::fromLatin1("background-color: %0").arg(colorName));
        lightSource->color = color;
        emit componentChanged(lightSource);
    }
}
