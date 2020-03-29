#include "environmentwidget.h"
#include "ecs/scene.h"
#include "globals.h"
#include "resources/texture.h"
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSignalBlocker>


EnvironmentWidget::EnvironmentWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(QString::fromLatin1("Environment"));

    auto vlayout = new QVBoxLayout;
    auto labelTexture = new QLabel("Texture");
    comboTexture = new QComboBox;
    auto hlayout = new QHBoxLayout;
    hlayout->addWidget(labelTexture);
    hlayout->addWidget(comboTexture);
    vlayout->addItem(hlayout);
    setLayout(vlayout);

    connect(comboTexture, SIGNAL(currentIndexChanged(int)), this, SLOT(onTextureChanged(int)));
}

void EnvironmentWidget::setEnvironment(Environment *env)
{
    environment = env;
    if (environment == nullptr) return;

    QSignalBlocker block(comboTexture);

    updateLayout();
}

void EnvironmentWidget::onTextureChanged(int index)
{
    environment->texture = (Texture*) comboTexture->itemData(index).value<void*>();
    environment->needsProcessing = true;
    emit componentChanged(environment);
}

void EnvironmentWidget::updateLayout()
{
    destroyLayout();

    comboTexture->addItem("None", QVariant::fromValue<void*>(nullptr));

    for (auto resource : resourceManager->resources)
    {
        auto *texture = resource->asTexture();

        if (texture != nullptr)
        {
            comboTexture->addItem(texture->name, QVariant::fromValue<void*>(texture));

            if (environment != nullptr && environment->texture == texture)
            {
                comboTexture->setCurrentIndex(comboTexture->count() - 1);
            }
        }
    }
}

void EnvironmentWidget::destroyLayout()
{
    comboTexture->clear();
}
