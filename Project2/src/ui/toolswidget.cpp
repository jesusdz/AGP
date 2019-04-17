#include "toolswidget.h"
#include "ui_toolswidget.h"
#include "globals.h"
#include <random>

ToolsWidget::ToolsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolsWidget)
{
    ui->setupUi(this);

    connect(ui->buttonGenerate, SIGNAL(clicked()), this, SLOT(onGenerateClicked()));
}

ToolsWidget::~ToolsWidget()
{
    delete ui;
}

void ToolsWidget::onGenerateClicked()
{
    const int countX = ui->spinCountX->value();
    const int countZ = ui->spinCountY->value();
    const float distX = ui->spinDistanceX->value();
    const float distZ = ui->spinDistanceY->value();
    const bool randomizePositions = ui->checkBoxRandomizePositions->isChecked();
    const bool randomizeRotations = ui->checkBoxRandomizeRotations->isChecked();

    std::uniform_real_distribution<float> randomFloats(-1.0, 1.0);
    std::default_random_engine generator;

    if (selection->count == 0) return;
    Entity *entity = selection->entities[0];

    for (int x = 0; x < countX; ++x)
    {
        for (int z = 0; z < countZ; ++z)
        {
            if (x == 0 && z == 0) continue;

            Entity *cloned = entity->clone();

            cloned->transform->position += QVector3D(distX * x, 0.0, distZ * z);

            if (randomizePositions)
            {
                cloned->transform->position += QVector3D(distX * 0.25 * randomFloats(generator), 0.0, distZ * 0.5 * randomFloats(generator));
            }

            if (randomizeRotations)
            {
                cloned->transform->rotation = QQuaternion::fromEulerAngles(0.0, randomFloats(generator) * 180.0f, 0.0);
            }
        }
    }
}
