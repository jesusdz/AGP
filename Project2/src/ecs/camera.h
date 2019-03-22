#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>

static const double DEFAULT_CAMERA_SPEED = 4.0;
static const double DEFAULT_CAMERA_FOVY = 60.0;

class Camera
{
public:
    Camera();

    // Returns true if the camera changed
    bool preUpdate();

    // Viewport
    void setViewportSize(int width, int height);

    // Create the matrices
    void prepareMatrices();


    // Camera parameters
    float cyaw = 0.0f;
    float cpitch = 0.0f;
    QVector3D cpos;
    int viewportWidth = 128;
    int viewportHeight = 128;
    float fovy = DEFAULT_CAMERA_FOVY;

    float speed = DEFAULT_CAMERA_SPEED;

    QMatrix4x4 worldMatrix; // From camera space to world space
    QMatrix4x4 viewMatrix; // From world space to camera space
    QMatrix4x4 projectionMatrix; // From view space to clip space
};

#endif // CAMERA_H
