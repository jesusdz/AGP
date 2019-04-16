#ifndef ENVIRONMENTWIDGET_H
#define ENVIRONMENTWIDGET_H

#include <QWidget>

class Component;
class Environment;
class QComboBox;

class EnvironmentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EnvironmentWidget(QWidget *parent = nullptr);

    void setEnvironment(Environment *env);

signals:

    void componentChanged(Component *);

public slots:

    void onTextureChanged(int index);

private:

    void updateLayout();
    void destroyLayout();

    Environment *environment = nullptr;

    QComboBox *comboTexture = nullptr;
};

#endif // ENVIRONMENTWIDGET_H
