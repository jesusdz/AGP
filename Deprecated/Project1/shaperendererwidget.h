#ifndef SHAPERENDERERWIDGET_H
#define SHAPERENDERERWIDGET_H

#include <QWidget>
#include <QComboBox>

class QDoubleSpinBox;
class QPushButton;
class ShapeRenderer;
class Component;

class ShapeRendererWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ShapeRendererWidget(QWidget *parent = nullptr);
    ~ShapeRendererWidget();

    void setShapeRenderer(ShapeRenderer *s);

signals:

    void componentChanged(Component *);

public slots:

    void onShapeChanged(int index);
    void onSizeChanged(double size);
    void onButtonFillColorPressed();
    void onButtonStrokeColorPressed();
    void onStrokeThicknessChanged(double size);
    void onStrokeStyleChanged(int index);


private:

    QComboBox *comboShape;
    QDoubleSpinBox *spinboxSize;
    QPushButton *buttonFillColor;
    QPushButton *buttonStrokeColor;
    QDoubleSpinBox *spinboxStrokeThickness;
    QComboBox *comboStrokeStyle;
    ShapeRenderer *shapeRenderer = nullptr;
};

#endif // SHAPERENDERERWIDGET_H
