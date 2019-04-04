#ifndef CAMERA_H
#define CAMERA_H

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>

static const float DEFAULT_CAMERA_SPEED = 4.0f;
static const float DEFAULT_CAMERA_FOVY = 60.0f;
static const float DEFAULT_CAMERA_ZNEAR = 0.01f;
static const float DEFAULT_CAMERA_ZFAR = 10000.0f;

class Camera
{
public:
    Camera();

    // Viewport
    void setViewportSize(int width, int height);

    QVector4D getLeftRightBottomTop();

    QVector3D screenPointToRay(int x, int y);
    QVector3D screenDisplacementToWorldVector(int x0, int y0, int x1, int y1, const QVector3D &worldPoint);
    QVector2D worldToScreenPoint(const QVector3D &p);

    // Create the matrices
    void prepareMatrices();


    // Placement parameters
    float cyaw = 0.0f;
    float cpitch = 0.0f;
    QVector3D cpos;

    // Viewport parameters
    int viewportWidth = 128;
    int viewportHeight = 128;

    // Projection parameters
    float fovy = DEFAULT_CAMERA_FOVY;
    float znear = DEFAULT_CAMERA_ZNEAR;
    float zfar = DEFAULT_CAMERA_ZFAR;

    // Speed
    float speed = DEFAULT_CAMERA_SPEED;

    // Matrices
    QMatrix4x4 worldMatrix; // From camera space to world space
    QMatrix4x4 viewMatrix; // From world space to camera space
    QMatrix4x4 projectionMatrix; // From view space to clip space
};

#endif // CAMERA_H
