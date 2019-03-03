#include "ui/resourcewidget.h"
#include "ui_resourcewidget.h"
#include "resources/resource.h"

ResourceWidget::ResourceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResourceWidget)
{
    ui->setupUi(this);

    connect(ui->nameText, SIGNAL(textEdited(const QString &)), this, SLOT(onNameChanged(const QString &)));
}

ResourceWidget::~ResourceWidget()
{
    delete ui;
}

void ResourceWidget::setResource(Resource *r)
{
    resource = r;
    if (resource == nullptr) return;

    ui->nameText->setText(r->name);
}

void ResourceWidget::onNameChanged(const QString &name)
{
    resource->name = name;
    emit resourceChanged(resource);
}
