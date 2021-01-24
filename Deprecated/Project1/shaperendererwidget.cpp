#include "shaperendererwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QColorDialog>
#include <QPushButton>
#include "scene.h"

ShapeRendererWidget::ShapeRendererWidget(QWidget *parent) :
    QWidget(parent)
{
    QFont font;
    font.setBold(true);

    setWindowTitle(QString::fromLatin1("Shape Renderer"));

    auto vlayout = new QVBoxLayout;

    { // Shape
        auto label = new QLabel;
        label->setText("Shape");

        comboShape = new QComboBox;
        comboShape->addItem("Square");
        comboShape->addItem("Circle");
        comboShape->addItem("Triangle");

        auto hlayout = new QHBoxLayout;
        hlayout->addWidget(label);
        hlayout->addWidget(comboShape);
        vlayout->addItem(hlayout);
    }

    { // Size
        auto label = new QLabel;
        label->setText("Size");

        spinboxSize = new QDoubleSpinBox;

        auto hlayout = new QHBoxLayout;
        hlayout->addWidget(label);
        hlayout->addWidget(spinboxSize);
        vlayout->addItem(hlayout);
    }

    { // Fill color
        auto label = new QLabel;
        label->setText("Fill color");

        auto colorButton = new QPushButton;
        colorButton->setText("");
        colorButton->setStyleSheet("background-color: #ffffff;");
        buttonFillColor = colorButton;

        auto hlayout = new QHBoxLayout;
        hlayout->addWidget(label);
        hlayout->addWidget(colorButton);
        vlayout->addItem(hlayout);
    }

    { // Stroke color
        auto label = new QLabel;
        label->setText("Stroke color");

        auto colorButton = new QPushButton;
        colorButton->setText("");
        colorButton->setStyleSheet("background-color: #ffffff;");
        buttonStrokeColor = colorButton;

        auto hlayout = new QHBoxLayout;
        hlayout->addWidget(label);
        hlayout->addWidget(colorButton);
        vlayout->addItem(hlayout);
    }

    { // Stroke thickness
        auto label = new QLabel;
        label->setText("Stroke thickness");

        spinboxStrokeThickness = new QDoubleSpinBox;

        auto hlayout = new QHBoxLayout;
        hlayout->addWidget(label);
        hlayout->addWidget(spinboxStrokeThickness);
        vlayout->addItem(hlayout);
    }

    { // Stroke style
        auto label = new QLabel;
        label->setText("Stroke style");

        comboStrokeStyle = new QComboBox;
        comboStrokeStyle->addItem("Solid");
        comboStrokeStyle->addItem("Dashed");

        auto hlayout = new QHBoxLayout;
        hlayout->addWidget(label);
        hlayout->addWidget(comboStrokeStyle);
        vlayout->addItem(hlayout);
    }

    setLayout(vlayout);

    spinboxSize->setRange(1.0, 500.0);
    spinboxStrokeThickness->setRange(0.0, 20.0);

    connect(comboShape, SIGNAL(currentIndexChanged(int)), this, SLOT(onShapeChanged(int)));
    connect(spinboxSize, SIGNAL(valueChanged(double)), this, SLOT(onSizeChanged(double)));
    connect(buttonFillColor, SIGNAL(clicked()), this, SLOT(onButtonFillColorPressed()));
    connect(buttonStrokeColor, SIGNAL(clicked()), this, SLOT(onButtonStrokeColorPressed()));
    connect(spinboxStrokeThickness, SIGNAL(valueChanged(double)), this, SLOT(onStrokeThicknessChanged(double)));
    connect(comboStrokeStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(onStrokeStyleChanged(int)));
}

ShapeRendererWidget::~ShapeRendererWidget()
{
}

void ShapeRendererWidget::setShapeRenderer(ShapeRenderer *sr)
{
    shapeRenderer = sr;
    if (shapeRenderer == nullptr) return;

    comboShape->setCurrentIndex(int(sr->shape));
    spinboxSize->setValue(double(sr->size));
    buttonFillColor->setStyleSheet(QString("background-color: %0").arg(sr->fillColor.name()));
    buttonStrokeColor->setStyleSheet(QString("background-color: %0").arg(sr->strokeColor.name()));
    spinboxStrokeThickness->setValue(double(sr->strokeThickness));
    comboStrokeStyle->setCurrentIndex(int(sr->strokeStyle));
}

void ShapeRendererWidget::onShapeChanged(int index)
{
    shapeRenderer->shape = Shape(index);
    emit componentChanged(shapeRenderer);
}

void ShapeRendererWidget::onSizeChanged(double size)
{
    shapeRenderer->size = int(size);
    emit componentChanged(shapeRenderer);
}

void ShapeRendererWidget::onButtonFillColorPressed()
{
    QColor color = shapeRenderer->fillColor;
    color = QColorDialog::getColor(color);
    if (color.isValid())
    {
        buttonFillColor->setStyleSheet(QString("background-color: %0").arg(color.name()));
        shapeRenderer->fillColor = color;
        emit componentChanged(shapeRenderer);
    }
}

void ShapeRendererWidget::onButtonStrokeColorPressed()
{
    QColor color = shapeRenderer->strokeColor;
    color = QColorDialog::getColor(color);
    if (color.isValid())
    {
        buttonStrokeColor->setStyleSheet(QString("background-color: %0").arg(color.name()));
        shapeRenderer->strokeColor = color;
        emit componentChanged(shapeRenderer);
    }
}

void ShapeRendererWidget::onStrokeThicknessChanged(double size)
{
    shapeRenderer->strokeThickness = float(size);
    emit componentChanged(shapeRenderer);
}

void ShapeRendererWidget::onStrokeStyleChanged(int index)
{
    shapeRenderer->strokeStyle = StrokeStyle(index);
    emit componentChanged(shapeRenderer);
}
