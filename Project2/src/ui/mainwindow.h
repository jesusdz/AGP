#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class HierarchyWidget;
class ResourcesWidget;
class InspectorWidget;
class ResourceManager;
class Scene;
class Entity;
class Resource;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    void openProject();
    void saveProject();
    void saveScreenshot();
    void showAboutOpenGL();
    void addCube();
    void addPlane();
    void addSphere();
    void addPointLight();
    void addDirectionalLight();
    void addMesh();
    void addTexture();
    void addMaterial();
    void exit();
    void onEntityAdded(Entity *entity);
    void onEntityRemoved(Entity *entity);
    void onEntitySelected(Entity *entity);
    void onEntityChanged(Entity *entity);
    void onResourceAdded(Resource *resource);
    void onResourceRemoved(Resource *resource);
    void onResourceSelected(Resource *resource);
    void onResourceChanged(Resource *resource);
    void updateEverything();

private:

    void createPanelVisibilityAction(QDockWidget *widget);

    void closeEvent(QCloseEvent *event);

private:

    Ui::MainWindow *uiMainWindow;

public:
    HierarchyWidget *hierarchyWidget;
    ResourcesWidget *resourcesWidget;
    InspectorWidget *inspectorWidget;
};

extern MainWindow *g_MainWindow;

#endif // MAINWINDOW_H
