#include "interaction.h"
#include "globals.h"
#include "util/raycast.h"
#include <QtMath>

bool Interaction::update()
{
    bool changed = false;

    switch (state)
    {
    case State::Idle:
        changed = idle();
        break;

    case State::Navigating:
        changed = navigate();
        break;

    case State::Translating:
        changed = translate();
        break;

    case State::Rotating:
        changed = rotate();
        break;

    case State::Scaling:
        changed = scale();
        break;
    }

    return changed;
}

bool Interaction::idle()
{
    if (input->mouseButtons[Qt::RightButton] == MouseButtonState::Down)
    {
        state = State::Navigating;
    }
    else if (input->mouseButtons[Qt::LeftButton] == MouseButtonState::Pressed)
    {
        QVector3D rayWorldspace = camera->screenPointToRay(input->mousex, input->mousey);
        Entity *entity = nullptr;
        rayCast(camera->cpos, rayWorldspace, &entity);
        selection->select(entity);
        return true;
    }
    else if(selection->count > 0)
    {
        if (input->keys[Qt::Key_T] == KeyState::Pressed)
        {
            state = State::Translating;
        }
        else if (input->keys[Qt::Key_R] == KeyState::Pressed)
        {
            state = State::Rotating;
        }
        else if (input->keys[Qt::Key_S] == KeyState::Pressed)
        {
            state = State::Scaling;
        }
    }

    return false;
}

bool Interaction::navigate()
{
    if (input->mouseButtons[Qt::RightButton] != MouseButtonState::Down)
    {
        state = State::Idle;
        return false;
    }

    bool cameraChanged = false;

    int mousex_delta = input->mousex - input->mousex_prev;
    int mousey_delta = input->mousey - input->mousey_prev;

    QVector3D &cpos = camera->cpos;
    float &cyaw = camera->cyaw;
    float &cpitch = camera->cpitch;

    // Camera navigation
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

    displacementVector *= camera->speed;

    cpos += displacementVector / 60.0f;

    return cameraChanged;
}

bool Interaction::translate()
{
    const bool cancel = input->mouseButtons[Qt::RightButton] == MouseButtonState::Pressed;
    const bool apply  = input->mouseButtons[Qt::LeftButton] == MouseButtonState::Pressed;

    if (cancel || apply)
    {
        state = State::Idle;
        return false;
    }

//    int mousex_delta = input->mousex - input->mousex_prev;
//    int mousey_delta = input->mousey - input->mousey_prev;
//    const QVector3D center =
//            selection->entities[0]->transform->matrix() * QVector3D(0.0, 0.0, 0.0);

//    const QVector3D worldDisplacement = camera->screenDisplacementToWorldVector(mousex_delta, mousey_delta, center);

    return false;
}

bool Interaction::rotate()
{
    const bool cancel = input->mouseButtons[Qt::RightButton] == MouseButtonState::Pressed;
    const bool apply  = input->mouseButtons[Qt::LeftButton] == MouseButtonState::Pressed;

    if (cancel || apply)
    {
        state = State::Idle;
        return false;
    }

    return false;
}

bool Interaction::scale()
{
    const bool cancel = input->mouseButtons[Qt::RightButton] == MouseButtonState::Pressed;
    const bool apply  = input->mouseButtons[Qt::LeftButton] == MouseButtonState::Pressed;

    if (cancel || apply)
    {
        state = State::Idle;
        return false;
    }

    return false;
}
