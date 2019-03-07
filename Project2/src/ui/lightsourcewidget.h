#ifndef LIGHTWIDGET_H
#define LIGHTWIDGET_H

#include <QWidget>

class Component;
class LightSource;

class LightSourceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LightSourceWidget(QWidget *parent = nullptr);

    void setLightSource(LightSource *light);

signals:

    void componentChanged(Component *);

public slots:

private:

    LightSource *lightSource = nullptr;
};

#endif // LIGHTWIDGET_H
