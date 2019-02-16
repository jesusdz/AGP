#include "customwidget.h"
#include <QPainter>


CustomWidget::CustomWidget(QWidget *parent) : QWidget(parent)
{
    setAutoFillBackground(true);
    // update() call it whenever we want to repaint the widget
}

QSize CustomWidget::sizeHint() const
{
    return QSize(256, 256);
}

QSize CustomWidget::minimumSizeHint() const
{
    return QSize(64, 64);
}

void CustomWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QBrush brush(QColor::fromRgb(150,100, 120));
    QPen pen;

    painter.setBrush(brush);
    //painter.setPen(pen);
    painter.drawRect(rect());
}
