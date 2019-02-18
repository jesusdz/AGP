#include "mycustomwidget.h"
#include "scene.h"
#include <QPainter>


MyCustomWidget::MyCustomWidget(QWidget *parent) : QWidget(parent)
{
    setAutoFillBackground(true);
}

QSize MyCustomWidget::sizeHint() const
{
    return QSize(256, 256);
}

QSize MyCustomWidget::minimumSizeHint() const
{
    return QSize(64, 64);
}

// Call update() whenever we want to repaint the widget
void MyCustomWidget::paintEvent(QPaintEvent *)
{
    QColor blueColor = QColor::fromRgb(127,190,220);
    QColor whiteColor = QColor::fromRgb(255,255,255);
    QColor blackColor = QColor::fromRgb(0,0,0);

    // Prepare the painter for this widget
    QPainter painter(this);

    QBrush brush;
    QPen pen;

    // Brush/pen configuration
    brush.setColor(blueColor);
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    pen.setStyle(Qt::PenStyle::NoPen);

    for (int i = 0; i < g_Scene->numEntities(); ++i)
    {
        BackgroundRenderer *bgr = g_Scene->entityAt(i)->backgroundRenderer;
        if (bgr != nullptr)
        {
            brush.setColor(bgr->color);
            break;
        }
    }

    painter.setBrush(brush);
    painter.setPen(pen);

    // Paint background
    painter.drawRect(rect());

    // Brush/pen configuration
    brush.setColor(whiteColor);
    pen.setWidth(4);
    pen.setColor(blackColor);
    pen.setStyle(Qt::PenStyle::DashLine);
    painter.setBrush(brush);
    painter.setPen(pen);

    // Draw circle
    int r = 64;
    int w = r * 2;
    int h = r * 2;
    int x = rect().width() / 2 - r;
    int y = rect().height() / 2 - r;
    QRect circleRect(x, y, w, h);
    painter.drawEllipse(circleRect);
}
