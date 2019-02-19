#ifndef INSPECTORWIDGET_H
#define INSPECTORWIDGET_H

#include <QWidget>

class QPushButton;
class Entity;
class Component;
class EntityWidget;
class TransformWidget;
class ShapeRendererWidget;
class BackgroundRendererWidget;
class ComponentWidget;

class InspectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InspectorWidget(QWidget *parent = nullptr);
    ~InspectorWidget();

    void showEntity(Entity *e);

public slots:

    void onEntityChanged(Entity *);
    void onComponentChanged(Component *);
    void onAddShapeRendererClicked();
    void onAddBackgroundRendererClicked();
    void onRemoveComponent(Component *);

signals:

    void entityChanged(Entity *e);

private:

    void updateLayout();

    Entity *entity = nullptr;
    QLayout *layout = nullptr;
    EntityWidget *entityWidget = nullptr;
    TransformWidget *transformWidget = nullptr;
    ShapeRendererWidget *shapeRendererWidget = nullptr;
    BackgroundRendererWidget *backgroundRendererWidget = nullptr;
    ComponentWidget *transformComponentWidget = nullptr;
    ComponentWidget *shapeRendererComponentWidget = nullptr;
    ComponentWidget *backgroundRendererComponentWidget = nullptr;
    QPushButton *buttonAddShapeRenderer = nullptr;
    QPushButton *buttonAddBackgroundRenderer = nullptr;
};

#endif // INSPECTORWIDGET_H
