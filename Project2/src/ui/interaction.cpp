#include "interaction.h"
#include "globals.h"
#include "util/raycast.h"
#include <QtMath>

bool Interaction::update()
{
    bool cameraChanged = false;

    int mousex_delta = input->mousex - input->mousex_prev;
    int mousey_delta = input->mousey - input->mousey_prev;

    QVector3D &cpos = camera->cpos;
    float &cyaw = camera->cyaw;
    float &cpitch = camera->cpitch;

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
        displacementVector *= camera->speed;

        cpos += displacementVector / 60.0f;
    }
    // Selection
    else if (input->mouseButtons[Qt::LeftButton] == MouseButtonState::Pressed)
    {
        QVector3D rayWorldspace = camera->screenPointToRay(input->mousex, input->mousey);
        Entity *entity = nullptr;
        rayCast(camera->cpos, rayWorldspace, &entity);
        selection->select(entity);
    }

    return cameraChanged;
}
