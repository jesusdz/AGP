#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class HierarchyWidget;
class InspectorWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    void openProject();
    void saveProject();

private:
    Ui::MainWindow *uiMainWindow;
    HierarchyWidget *hierarchyWidget;
    InspectorWidget *inspectorWidget;
};

#endif // MAINWINDOW_H
