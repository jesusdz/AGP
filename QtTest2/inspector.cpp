#include "inspector.h"
#include "transform.h"
#include "meshrenderer.h"
#include "customwidget.h"
#include <QLayout>
#include <QVBoxLayout>
#include <QSpacerItem>

Inspector::Inspector(QWidget *parent) :
    QWidget(parent)
{
    Transform *transform = new Transform;
    MeshRenderer *meshRenderer = new MeshRenderer;
    CustomWidget *customWidget = new CustomWidget;
    QSpacerItem *spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);


    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(transform);
    layout->addWidget(meshRenderer);
    layout->addWidget(customWidget);
    layout->addItem(spacer);
    setLayout(layout);
}

Inspector::~Inspector()
{
}
