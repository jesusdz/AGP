#include "texturewidget.h"
#include "ui_texturewidget.h"
#include "resources/texture.h"
#include <QFileDialog>


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

void TextureWidget::setTexture(Texture *t)
{
    texture = t;
    ui->openGLWidget->setTexture(t);
}

void TextureWidget::onButtonClicked()
{
    if (texture == nullptr) return;

    QString path = QFileDialog::getOpenFileName(this,"Load image file", QString(), QString::fromLatin1("Image files (*.png *.jpg *.gif *.bmp)"));
    if (!path.isEmpty())
    {
        texture->loadTexture(path.toLatin1());
        emit resourceChanged(texture);
    }
}
