#include "texturewidget.h"
#include "ui_texturewidget.h"
#include "resources/texture.h"
#include "resources/resourcemanager.h"
#include "globals.h"

TextureWidget::TextureWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TextureWidget)
{
    ui->setupUi(this);
    connect(ui->buttonOpen, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
}

TextureWidget::~TextureWidget()
{
    delete ui;
}

void TextureWidget::setTexture(Texture *m)
{
    texture = m;
}

void TextureWidget::onButtonClicked()
{
    if (texture == nullptr) return;

    texture->loadTexture(":/icons/save_screenshot");

    emit resourceChanged(texture);
}
