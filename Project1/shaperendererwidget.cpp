#include "shaperendererwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "scene.h"

ShapeRendererWidget::ShapeRendererWidget(QWidget *parent) :
    QWidget(parent)
{
    QFont font;
    font.setBold(true);

    setWindowTitle(QString::fromLatin1("Shape Renderer"));

    auto label = new QLabel;
    label->setText("Mesh");

    comboShape = new QComboBox;
    comboShape->addItem("Square");
    comboShape->addItem("Circle");
    comboShape->addItem("Triangle");

    auto hlayout = new QHBoxLayout;
    hlayout->addWidget(label);
    hlayout->addWidget(comboShape);

    auto vlayout = new QVBoxLayout;
    vlayout->addItem(hlayout);

    setLayout(vlayout);
}

ShapeRendererWidget::~ShapeRendererWidget()
{
}

void ShapeRendererWidget::setShapeRenderer(ShapeRenderer *sr)
{
    shapeRenderer = sr;
    if (shapeRenderer == nullptr) return;

    // TODO
}
