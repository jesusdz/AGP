#include "miscsettingswidget.h"
#include "ui_miscsettingswidget.h"
#include "globals.h"

MiscSettingsWidget::MiscSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MiscSettingsWidget)
{
    ui->setupUi(this);

    ui->spinCameraSpeed->setValue(DEFAULT_CAMERA_SPEED);
    ui->spinFovY->setValue(DEFAULT_CAMERA_FOVY);

    connect(ui->spinCameraSpeed, SIGNAL(valueChanged(double)), this, SLOT(onCameraSpeedChanged(double)));
    connect(ui->spinFovY, SIGNAL(valueChanged(double)), this, SLOT(onCameraFovYChanged(double)));
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
