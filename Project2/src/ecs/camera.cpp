#include "camera.h"
#include <QtMath>
#include <QMatrix3x3>


Camera::Camera()
{
    // Camera position
    cpos = QVector3D(0.0, 2.0, 6.0);
}

void Camera::setViewportSize(int width, int height)
{
    viewportWidth = width;
    viewportHeight = height;
}

QVector4D Camera::getLeftRightBottomTop()
{
    const float aspectRatio = float(viewportWidth) / viewportHeight;
    const float alpha = qDegreesToRadians(fovy * 0.5);
    const float top = znear * qTan(alpha);
    const float bottom = -top;
    const float right = top * aspectRatio;
    const float left = -right;
    QVector4D params(left, right, bottom, top);
    return params;
}

QVector3D Camera::screenPointToRay(int x, int y)
{
    QVector4D lrbt = getLeftRightBottomTop();
    const float l = lrbt.x();
    const float r = lrbt.y();
    const float b = lrbt.z();
    const float t = lrbt.w();
    const float rayX = l + (r - l) * x / viewportWidth;
    const float rayY = b + (t - b) * (viewportHeight - y - 1) / viewportHeight;
    const float rayZ = -znear;
    QVector3D rayViewspace = QVector3D(rayX, rayY, rayZ);

    prepareMatrices();
    QVector3D rayWorldspace = QVector3D(worldMatrix * QVector4D(rayViewspace, 0.0));

    return rayWorldspace;
}

void Camera::prepareMatrices()
{
    worldMatrix.setToIdentity();
    worldMatrix.translate(cpos);
    worldMatrix.rotate(cyaw, QVector3D(0.0, 1.0, 0.0));
    worldMatrix.rotate(cpitch, QVector3D(1.0, 0.0, 0.0));

    viewMatrix = worldMatrix.inverted();

    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(fovy, float(viewportWidth) / viewportHeight, 0.01, 1000.0);
}
