#include "ui/entitywidget.h"
#include "ui_entitywidget.h"
#include "ecs/scene.h"

EntityWidget::EntityWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EntityWidget)
{
    ui->setupUi(this);

    connect(ui->nameText, SIGNAL(textEdited(const QString &)), this, SLOT(onNameChanged(const QString &)));
    connect(ui->nameText, SIGNAL(returnPressed()), this, SLOT(clearFocus()));
}

EntityWidget::~EntityWidget()
{
    delete ui;
}

void EntityWidget::setEntity(Entity *ent)
{
    entity = ent;
    if (entity == nullptr) return;

    if (ent->name != ui->nameText->text()) {
        ui->nameText->setText(ent->name);
    }
}

void EntityWidget::onNameChanged(const QString &name)
{
    entity->name = name;
    emit entityChanged(entity);
}

void EntityWidget::clearFocus()
{
    ui->nameText->clearFocus();
}
