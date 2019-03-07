#include "lightsourcewidget.h"

LightSourceWidget::LightSourceWidget(QWidget *parent) : QWidget(parent)
{

}

void LightSourceWidget::setLightSource(LightSource *light)
{
    lightSource = light;
}
