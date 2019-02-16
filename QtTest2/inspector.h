#ifndef INSPECTOR_H
#define INSPECTOR_H

#include <QWidget>

class Inspector : public QWidget
{
    Q_OBJECT

public:
    explicit Inspector(QWidget *parent = nullptr);
    ~Inspector();

};

#endif // INSPECTOR_H
