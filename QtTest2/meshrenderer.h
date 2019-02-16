#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include <QWidget>
#include <QComboBox>

class MeshRenderer : public QWidget
{
    Q_OBJECT

public:
    explicit MeshRenderer(QWidget *parent = nullptr);
    ~MeshRenderer();

private:

    QComboBox *comboMesh;
};

#endif // MESHRENDERER_H
