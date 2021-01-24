#include "miscsettingswidget.h"
#include "ui_miscsettingswidget.h"
#include "globals.h"
#include <QColorDialog>


MiscSettingsWidget::MiscSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MiscSettingsWidget)
{
    ui->setupUi(this);

    ui->spinCameraSpeed->setValue(DEFAULT_CAMERA_SPEED);
    ui->spinFovY->setValue(DEFAULT_CAMERA_FOVY);

    connect(ui->spinCameraSpeed, SIGNAL(valueChanged(double)), this, SLOT(onCameraSpeedChanged(double)));
    connect(ui->spinFovY, SIGNAL(valueChanged(double)), this, SLOT(onCameraFovYChanged(double)));
    connect(ui->spinMaxSubmeshes, SIGNAL(valueChanged(int)), this, SLOT(onMaxSubmeshesChanged(int)));
    connect(ui->buttonBackgroundColor, SIGNAL(clicked()), this, SLOT(onBackgroundColorClicked()));
    connect(ui->groupBloom, SIGNAL(clicked()), this, SLOT(onVisualHintChanged()));
    connect(ui->spinBloomKernelRadius, SIGNAL(valueChanged(int)), this, SLOT(onVisualHintChanged(int)));
    connect(ui->sliderBloomLod0Intensity, SIGNAL(valueChanged(int)), this, SLOT(onVisualHintChanged(int)));
    connect(ui->sliderBloomLod1Intensity, SIGNAL(valueChanged(int)), this, SLOT(onVisualHintChanged(int)));
    connect(ui->sliderBloomLod2Intensity, SIGNAL(valueChanged(int)), this, SLOT(onVisualHintChanged(int)));
    connect(ui->sliderBloomLod3Intensity, SIGNAL(valueChanged(int)), this, SLOT(onVisualHintChanged(int)));
    connect(ui->sliderBloomLod4Intensity, SIGNAL(valueChanged(int)), this, SLOT(onVisualHintChanged(int)));
    connect(ui->checkBoxSSAO, SIGNAL(clicked()), this, SLOT(onVisualHintChanged()));
    connect(ui->checkBoxWater, SIGNAL(clicked()), this, SLOT(onVisualHintChanged()));
    connect(ui->checkBoxGrid, SIGNAL(clicked()), this, SLOT(onVisualHintChanged()));
    connect(ui->checkBoxLightSources, SIGNAL(clicked()), this, SLOT(onVisualHintChanged()));
    connect(ui->checkBoxSelectionOutline, SIGNAL(clicked()), this, SLOT(onVisualHintChanged()));
}

MiscSettingsWidget::~MiscSettingsWidget()
{
    delete ui;
}

void MiscSettingsWidget::onCameraSpeedChanged(double speed)
{
    camera->speed = speed;
}

void MiscSettingsWidget::onCameraFovYChanged(double fovy)
{
    camera->fovy = fovy;
    emit settingsChanged();
}

int g_MaxSubmeshes = 100;

void MiscSettingsWidget::onMaxSubmeshesChanged(int n)
{
    g_MaxSubmeshes = n;
    emit settingsChanged();
}

void MiscSettingsWidget::onBackgroundColorClicked()
{
    QColor color = QColorDialog::getColor(scene->backgroundColor, this, "Background color");
    if (color.isValid())
    {
        QString colorName = color.name();
        ui->buttonBackgroundColor->setStyleSheet(QString::fromLatin1("background-color: %0").arg(colorName));
        scene->backgroundColor = color;
        scene->environmentChanged = true;
        emit settingsChanged();
    }
}

void MiscSettingsWidget::onVisualHintChanged()
{
    scene->renderBloom = ui->groupBloom->isChecked();
    scene->bloomRadius = float(ui->spinBloomKernelRadius->value());
    scene->bloomLod0Intensity = float(ui->sliderBloomLod0Intensity->value())/100.0;
    scene->bloomLod1Intensity = float(ui->sliderBloomLod1Intensity->value())/100.0;
    scene->bloomLod2Intensity = float(ui->sliderBloomLod2Intensity->value())/100.0;
    scene->bloomLod3Intensity = float(ui->sliderBloomLod3Intensity->value())/100.0;
    scene->bloomLod4Intensity = float(ui->sliderBloomLod4Intensity->value())/100.0;
    scene->renderWater = ui->checkBoxWater->isChecked();
    scene->renderSSAO = ui->checkBoxSSAO->isChecked();
    scene->renderGrid = ui->checkBoxGrid->isChecked();
    scene->renderLightSources = ui->checkBoxLightSources->isChecked();
    scene->renderSelectionOutline = ui->checkBoxSelectionOutline->isChecked();
    emit settingsChanged();
}

void MiscSettingsWidget::onVisualHintChanged(int)
{
    onVisualHintChanged();
}
