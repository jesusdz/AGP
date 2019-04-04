#include "interaction.h"
#include "globals.h"
#include "util/raycast.h"
#include <QtMath>
#include <QVector2D>


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
    static QVector3D positionBackup;
    static bool idle = true;
    if (idle) {
        idle = false;
        positionBackup = selection->entities[0]->transform->position;
    }

    const int x0 = input->mousex_prev;
    const int y0 = input->mousey_prev;
    const int x1 = input->mousex;
    const int y1 = input->mousey;
    const QVector3D worldDisplacement = camera->screenDisplacementToWorldVector(x0, y0, x1, y1, selection->entities[0]->transform->position);

    selection->entities[0]->transform->position += worldDisplacement;

    const bool cancel = input->mouseButtons[Qt::RightButton] == MouseButtonState::Pressed;
    const bool apply  = input->mouseButtons[Qt::LeftButton] == MouseButtonState::Pressed;
    if (apply) { state = State::Idle; }
    if (cancel) { state = State::Idle; selection->entities[0]->transform->position = positionBackup; }
    idle = cancel || apply;

    return true;
}

bool Interaction::rotate()
{
    const QVector2D mousePos = QVector2D(input->mousex, input->mousey);
    const QVector2D centerPos = camera->worldToScreenPoint(selection->entities[0]->transform->position);

    static QQuaternion rotationBackup;
    static QVector2D initialMousePos;
    static bool idle = true;
    if (idle) {
        idle = false;
        rotationBackup = selection->entities[0]->transform->rotation;
        initialMousePos = mousePos;
    }

    const QVector2D initialVector = (initialMousePos - centerPos).normalized();
    const QVector2D currentVector = (mousePos - centerPos).normalized();
    float angle = qAcos(QVector2D::dotProduct(initialVector, currentVector));
    angle *= QVector3D::crossProduct(currentVector.toVector3D(), initialVector.toVector3D()).z() > 0.0?1.0f:-1.0f;
    const QVector3D axis = (camera->cpos - selection->entities[0]->transform->position).normalized();

    selection->entities[0]->transform->rotation = QQuaternion::fromAxisAndAngle(axis, qRadiansToDegrees(angle)) * rotationBackup;

    const bool cancel = input->mouseButtons[Qt::RightButton] == MouseButtonState::Pressed;
    const bool apply  = input->mouseButtons[Qt::LeftButton] == MouseButtonState::Pressed;
    if (cancel) { state = State::Idle; selection->entities[0]->transform->rotation = rotationBackup; }
    if (apply)  { state = State::Idle; }
    idle = cancel || apply;

    return true;
}

bool Interaction::scale()
{
    const QVector2D mousePos = QVector2D(input->mousex, input->mousey);
    const QVector2D centerPos = camera->worldToScreenPoint(selection->entities[0]->transform->position);

    static QVector3D scaleBackup;
    static QVector2D initialMousePos;
    static bool idle = true;
    if (idle) {
        idle = false;
        scaleBackup = selection->entities[0]->transform->scale;
        initialMousePos = mousePos;
    }

    const float initialLength = (initialMousePos - centerPos).length();
    const float currentLength = (mousePos - centerPos).length();
    const float scaleFactor = currentLength / initialLength;

    selection->entities[0]->transform->scale = scaleBackup * scaleFactor;

    const bool cancel = input->mouseButtons[Qt::RightButton] == MouseButtonState::Pressed;
    const bool apply  = input->mouseButtons[Qt::LeftButton] == MouseButtonState::Pressed;
    if (cancel) { state = State::Idle; selection->entities[0]->transform->scale = scaleBackup; }
    if (apply)  { state = State::Idle; }
    idle = cancel || apply;

    return true;
}
