#include "meshrendererwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

MeshRendererWidget::MeshRendererWidget(QWidget *parent) :
    QWidget(parent)
{
    QFont font;
    font.setBold(true);

    auto title = new QLabel;
    title->setText("Mesh Renderer");
    title->setFont(font);

    auto label = new QLabel;
    label->setText("Mesh");

    auto combo = new QComboBox;
    combo->addItem("Mesh 1");
    combo->addItem("Mesh 2");
    combo->addItem("Mesh 3");

    auto hlayout = new QHBoxLayout;
    hlayout->addWidget(label);
    hlayout->addWidget(combo);

    auto vlayout = new QVBoxLayout;
    vlayout->addWidget(title);
    vlayout->addItem(hlayout);

    setLayout(vlayout);
}

MeshRendererWidget::~MeshRendererWidget()
{
}
