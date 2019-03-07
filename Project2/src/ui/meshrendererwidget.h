#ifndef MESHRENDERERWIDGET_H
#define MESHRENDERERWIDGET_H

#include <QWidget>

class MeshRenderer;
class Component;
class QComboBox;
class QListWidget;

class MeshRendererWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MeshRendererWidget(QWidget *parent = nullptr);
    ~MeshRendererWidget();

    void setMeshRenderer(MeshRenderer *s);

signals:

    void componentChanged(Component *);

public slots:

    void onMeshChanged(int index);
    void addMaterial();


private:

    QComboBox *comboMesh = nullptr;
    MeshRenderer *meshRenderer = nullptr;
    QListWidget *materialsList = nullptr;
};

#endif // MESHRENDERERWIDGET_H
