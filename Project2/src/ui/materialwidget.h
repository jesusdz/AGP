#ifndef MATERIALWIDGET_H
#define MATERIALWIDGET_H

#include <QWidget>

namespace Ui {
class MaterialWidget;
}

class Material;
class Resource;

class MaterialWidget : public QWidget
{
    Q_OBJECT

public:

    explicit MaterialWidget(QWidget *parent = nullptr);
    ~MaterialWidget();

    void setMaterial(Material *);

signals:

    void resourceChanged(Resource *);

public slots:

    void onButtonAlbedoChanged();
    void onSmoothnessChanged(int );

private:
    Ui::MaterialWidget *ui;

    Material *material = nullptr;
};

#endif // MATERIALWIDGET_H
