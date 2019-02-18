#ifndef COMPONENTWIDGET_H
#define COMPONENTWIDGET_H

#include <QWidget>

namespace Ui {
class ComponentWidget;
}

class ComponentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ComponentWidget(QWidget *parent = nullptr);
    ~ComponentWidget();

    void setWidget(QWidget *widget);

public slots:

    void collapse(bool c);
    void remove();

private:
    Ui::ComponentWidget *ui;
};

#endif // COMPONENTWIDGET_H
