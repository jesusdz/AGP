#include "camera.h"
#include "ui/input.h"
#include "globals.h"
#include <QtMath>


Camera::Camera()
{
    // Camera position
    cpos = QVector3D(0.0, 2.0, 6.0);
}

bool Camera::preUpdate()
{
    bool cameraChanged = false;

    int mousex_delta = input->mousex - input->mousex_prev;
    int mousey_delta = input->mousey - input->mousey_prev;

    // Camera rotation
    if (input->mouseButtons[Qt::RightButton] == MouseButtonState::Down)
    {
        if (mousex_delta != 0 || mousey_delta != 0)
        {
            cyaw -= 0.3f * mousex_delta;
            cpitch -= 0.3f * mousey_delta;
            while (cyaw < 0.0f) cyaw += 360.0f;
            while (cyaw > 360.0f) cyaw -= 360.0f;
            if (cpitch > 89.0f) cpitch = 89.0f;
            if (cpitch < -89.0f) cpitch = -89.0f;
            cameraChanged = true;
        }

        QVector3D displacementVector;

        if (input->keys[Qt::Key_A] == KeyState::Down) // Left
        {
            displacementVector -= QVector3D(cosf(qDegreesToRadians(cyaw)), 0.0f, -sinf(qDegreesToRadians(cyaw)));
            cameraChanged = true;
        }
        if (input->keys[Qt::Key_D] == KeyState::Down) // Right
        {
            displacementVector += QVector3D(cosf(qDegreesToRadians(cyaw)), 0.0f, -sinf(qDegreesToRadians(cyaw)));
            cameraChanged = true;
        }
        if (input->keys[Qt::Key_W] == KeyState::Down) // Front
        {
            displacementVector += QVector3D(-sinf(qDegreesToRadians(cyaw)) * cosf(qDegreesToRadians(cpitch)), sinf(qDegreesToRadians(cpitch)), -cosf(qDegreesToRadians(cyaw)) * cosf(qDegreesToRadians(cpitch)));
            cameraChanged = true;
        }
        if (input->keys[Qt::Key_S] == KeyState::Down) // Back
        {
            displacementVector -= QVector3D(-sinf(qDegreesToRadians(cyaw)) * cosf(qDegreesToRadians(cpitch)), sinf(qDegreesToRadians(cpitch)), -cosf(qDegreesToRadians(cyaw)) * cosf(qDegreesToRadians(cpitch)));
            cameraChanged = true;
        }

        // Increase speed
        displacementVector *= speed;

        cpos += displacementVector / 60.0f;
    }

    return cameraChanged;
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
