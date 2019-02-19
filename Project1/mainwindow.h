#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class HierarchyWidget;
class InspectorWidget;
class Scene;
class Entity;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    void openProject();
    void saveProject();
    void onEntityAdded(Entity *entity);
    void onEntityRemoved(Entity *entity);
    void onEntitySelected(Entity *entity);
    void onEntityChanged(Entity *entity);

private:
    Ui::MainWindow *uiMainWindow;

public:
    HierarchyWidget *hierarchyWidget;
    InspectorWidget *inspectorWidget;
    Scene *scene;
};

extern MainWindow *g_MainWindow;

#endif // MAINWINDOW_H
