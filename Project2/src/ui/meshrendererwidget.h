#ifndef MESHRENDERERWIDGET_H
#define MESHRENDERERWIDGET_H

#include <QWidget>

class MeshRenderer;
class Material;
class Component;
class QComboBox;
class QVBoxLayout;

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

    void updateLayout();
    void onMeshChanged(int index);
    void onMaterialChanged(int index);
    void addMaterial();


private:

    void destroyLayout();
    QComboBox *createMeshesCombo();
    QVBoxLayout* createMaterialsLayout();
    QComboBox * createComboForMaterial(Material*);

    QComboBox *comboMesh = nullptr;
    MeshRenderer *meshRenderer = nullptr;
    QVBoxLayout *materialsList = nullptr;
    QVector<QComboBox*> combosMaterial;
};

#endif // MESHRENDERERWIDGET_H
