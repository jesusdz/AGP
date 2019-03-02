#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class HierarchyWidget;
class InspectorWidget;
class ResourceManager;
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
    void saveScreenshot();
    void showAboutOpenGL();
    void addCube();
    void addPlane();
    void addSphere();
    void exit();
    void onEntityAdded(Entity *entity);
    void onEntityRemoved(Entity *entity);
    void onEntitySelected(Entity *entity);
    void onEntityChanged(Entity *entity);
    void updateEverything();

private:

    void createPanelVisibilityAction(QDockWidget *widget);

    void closeEvent(QCloseEvent *event);

private:

    Ui::MainWindow *uiMainWindow;

public:
    HierarchyWidget *hierarchyWidget;
    InspectorWidget *inspectorWidget;
};

extern MainWindow *g_MainWindow;

#endif // MAINWINDOW_H
