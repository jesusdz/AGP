#ifndef INSPECTORWIDGET_H
#define INSPECTORWIDGET_H

#include <QWidget>

class QPushButton;
class Entity;
class Component;
class EntityWidget;
class TransformWidget;
class MeshRendererWidget;
class ComponentWidget;
class MeshWidget;
class TextureWidget;
class MaterialWidget;
class Resource;
class ResourceWidget;

class InspectorWidget : public QWidget
{
    Q_OBJECT

public:

    explicit InspectorWidget(QWidget *parent = nullptr);
    ~InspectorWidget();

    void showEntity(Entity *e);
    void showResource(Resource *r);

public slots:

    void updateLayout();
    void onEntityChanged(Entity *);
    void onComponentChanged(Component *);
    void onAddMeshRendererClicked();
    void onRemoveComponent(Component *);
    void onResourceChanged(Resource *);

signals:

    void entityChanged(Entity *e);
    void resourceChanged(Resource *r);

private:

    Entity *entity = nullptr;
    QLayout *layout = nullptr;
    EntityWidget *entityWidget = nullptr;
    TransformWidget *transformWidget = nullptr;
    MeshRendererWidget *meshRendererWidget = nullptr;
    ComponentWidget *transformComponentWidget = nullptr;
    ComponentWidget *meshRendererComponentWidget = nullptr;
    QPushButton *buttonAddMeshRenderer = nullptr;

    Resource *resource = nullptr;
    ResourceWidget *resourceWidget = nullptr;
    MeshWidget *meshWidget = nullptr;
    TextureWidget *textureWidget = nullptr;
    MaterialWidget *materialWidget = nullptr;
};

#endif // INSPECTORWIDGET_H
