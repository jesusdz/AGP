#include "mainwindow.h"
#include <QApplication>

const char *stylesheet =
        "QFrame#frame { background-color: gray; }";
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    a.setStyleSheet(stylesheet);

    return a.exec();
}
