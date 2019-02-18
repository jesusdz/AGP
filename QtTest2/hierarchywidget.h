#ifndef HIERARCHYWIDGET_H
#define HIERARCHYWIDGET_H

#include <QWidget>

namespace Ui {
class HierarchyWidget;
}

class HierarchyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HierarchyWidget(QWidget *parent = nullptr);
    ~HierarchyWidget();

public slots:

    void addEntity();
    void removeEntity();

private:
    Ui::HierarchyWidget *ui;
};

#endif // HIERARCHYWIDGET_H
