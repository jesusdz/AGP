#ifndef LIGHTWIDGET_H
#define LIGHTWIDGET_H

#include <QWidget>

class Component;
class LightSource;
class QComboBox;
class QDoubleSpinBox;
class QPushButton;
class QCheckBox;

class LightSourceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LightSourceWidget(QWidget *parent = nullptr);

    void setLightSource(LightSource *light);

signals:

    void componentChanged(Component *);

public slots:

    void onTypeChanged(int comboIndex);
    void onIntensityChanged(double value);
    void onRangeChanged(double value);
    void onColorButtonClicked();
    void onCastsShadowsChanged(Qt::CheckState);

private:

    LightSource *lightSource = nullptr;

    QComboBox *comboType = nullptr;
    QDoubleSpinBox *spinIntensity = nullptr;
    QDoubleSpinBox *spinRange = nullptr;
    QPushButton *buttonColor = nullptr;
    QCheckBox *checkCastsShadows = nullptr;
};

#endif // LIGHTWIDGET_H
