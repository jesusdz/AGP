#ifndef MESHRENDERERWIDGET_H
#define MESHRENDERERWIDGET_H

#include <QWidget>
#include <QComboBox>

class MeshRendererWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MeshRendererWidget(QWidget *parent = nullptr);
    ~MeshRendererWidget();

private:

    QComboBox *comboMesh;
};

#endif // MESHRENDERERWIDGET_H
