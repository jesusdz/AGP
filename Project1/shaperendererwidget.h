#ifndef SHAPERENDERERWIDGET_H
#define SHAPERENDERERWIDGET_H

#include <QWidget>
#include <QComboBox>

class ShapeRenderer;
class Component;

class ShapeRendererWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ShapeRendererWidget(QWidget *parent = nullptr);
    ~ShapeRendererWidget();

    void setShapeRenderer(ShapeRenderer *s);

signals:

    void componentChanged(Component *);

private:

    QComboBox *comboShape;
    ShapeRenderer *shapeRenderer = nullptr;
};

#endif // SHAPERENDERERWIDGET_H
