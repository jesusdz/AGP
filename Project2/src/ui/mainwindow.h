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
    void closeProject();
    void saveScreenshot();
    void showAboutOpenGL();
    void addCube();
    void addPlane();
    void addSphere();
    void addTerrain();
    void addPointLight();
    void addDirectionalLight();
    void importModel();
    void addMesh();
    void addTexture();
    void addMaterial();
    void exit();
    void onSceneChanged();
    void onEntityAdded(Entity *entity);
    void onEntityRemoved(Entity *entity);
    void onEntitySelectedFromHierarchy(Entity *entity);
    void onEntitySelectedFromSceneView(Entity *entity);
    void onEntityChanged(Entity *entity);
    void onResourceAdded(Resource *resource);
    void onResourceRemoved(Resource *resource);
    void onResourceSelected(Resource *resource);
    void onResourceChanged(Resource *resource);
    void updateRenderList();
    void updateRender();
    void updateEverything();
    void reloadShaderPrograms();
    void onRenderOutputChanged(QString);

private:

    void createPanelVisibilityAction(QDockWidget *widget);
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void closeEvent(QCloseEvent *event) override;

private:

    Ui::MainWindow *uiMainWindow;

public:
    HierarchyWidget *hierarchyWidget;
    ResourcesWidget *resourcesWidget;
    InspectorWidget *inspectorWidget;
};

extern MainWindow *g_MainWindow;

#endif // MAINWINDOW_H
