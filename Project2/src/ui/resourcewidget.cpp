#include "ui/resourcewidget.h"
#include "ui_resourcewidget.h"
#include "resources/resource.h"

ResourceWidget::ResourceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResourceWidget)
{
    ui->setupUi(this);

    connect(ui->nameText, SIGNAL(textEdited(const QString &)), this, SLOT(onNameChanged(const QString &)));
    connect(ui->nameText, SIGNAL(returnPressed()), this, SLOT(clearFocus()));
}

ResourceWidget::~ResourceWidget()
{
    delete ui;
}

void ResourceWidget::setResource(Resource *r)
{
    resource = r;
    if (resource == nullptr) return;

    if (r->name != ui->nameText->text()) {
        ui->nameText->setText(r->name);
    }
}

void ResourceWidget::onNameChanged(const QString &name)
{
    resource->name = name;
    emit resourceChanged(resource);
}

void ResourceWidget::clearFocus()
{
    ui->nameText->clearFocus();
}
