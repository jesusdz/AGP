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
    brush.setColor(whiteColor);
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


    // Paint entities
    for (int i = 0; i < g_Scene->numEntities(); ++i)
    {
        Transform *t = g_Scene->entityAt(i)->transform;
        ShapeRenderer *s = g_Scene->entityAt(i)->shapeRenderer;
        if (s != nullptr && t != nullptr)
        {
            // Brush/pen configuration
            brush.setColor(s->fillColor);
            if (int(s->strokeThickness) > 0.0)
            {
                brush.setStyle(Qt::BrushStyle::SolidPattern);
                pen.setWidthF(s->strokeThickness);
                if (s->strokeStyle == StrokeStyle::Dashed)
                {
                    pen.setStyle(Qt::PenStyle::DashLine);
                }
                else
                {
                    pen.setStyle(Qt::PenStyle::SolidLine);
                }
            }
            else
            {
                pen.setStyle(Qt::PenStyle::NoPen);
            }
            pen.setColor(s->strokeColor);
            painter.setBrush(brush);
            painter.setPen(pen);

            // Draw circle
            float w = t->sx * s->size;
            float h = t->sy * s->size;
            float rx = w / 2;
            float ry = h / 2;
            float x = t->tx - rx;
            float y = t->ty - ry;

            QRect rect(x, y, w, h);
            if (s->shape == Shape::Circle)
            {
                painter.drawEllipse(rect);
            }
            else if (s->shape == Shape::Square)
            {
                painter.drawRect(rect);
            }
            else if (s->shape == Shape::Triangle)
            {
                QPoint points[3] = {
                    QPoint(rect.x(), rect.y()),
                    QPoint(rect.x() + w, rect.y()),
                    QPoint(rect.x() + rx, rect.y() + h) };
                painter.drawPolygon(points, 3);
            }
        }
    }
}
