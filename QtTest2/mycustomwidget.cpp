#include "mycustomwidget.h"
#include <QPainter>


MyCustomWidget::MyCustomWidget(QWidget *parent) : QWidget(parent)
{
    setAutoFillBackground(true);
    // update() call it whenever we want to repaint the widget
}

QSize MyCustomWidget::sizeHint() const
{
    return QSize(256, 256);
}

QSize MyCustomWidget::minimumSizeHint() const
{
    return QSize(64, 64);
}

void MyCustomWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QBrush brush(QColor::fromRgb(150,100, 120));
    QPen pen;

    painter.setBrush(brush);
    //painter.setPen(pen);
    painter.drawRect(rect());
}
