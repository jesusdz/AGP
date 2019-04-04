#include "input.h"
#include <QKeyEvent>
#include <QMouseEvent>


Input::Input()
{
    for (int i = 0; i < MAX_BUTTONS; ++i) {
        mouseButtons[i] = MouseButtonState::Up;
    }
    for (int i = 0; i < MAX_KEYS; ++i) {
        keys[i] = KeyState::Up;
    }
}

void Input::keyPressEvent(QKeyEvent *event)
{
    if (event->key() < MAX_KEYS && !event->isAutoRepeat()) {
        if (keys[event->key()] == KeyState::Up) {
            keys[event->key()] = KeyState::Pressed;
        }
    }
}

void Input::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() < MAX_KEYS && !event->isAutoRepeat()) {
        keys[event->key()] = KeyState::Up;
    }
}

void Input::mousePressEvent(QMouseEvent *event)
{
    if (event->button() < MAX_BUTTONS) {
        if (mouseButtons[event->button()] == MouseButtonState::Up) {
            mousex = mousex_prev = event->x();
            mousey = mousey_prev = event->y();
            mouseButtons[event->button()] = MouseButtonState::Pressed;
        }
    }
}

void Input::mouseMoveEvent(QMouseEvent *event)
{
    mousex = event->x();
    mousey = event->y();
}

void Input::mouseReleaseEvent(QMouseEvent *event)
{
    mouseButtons[event->button()] = MouseButtonState::Up;
}

void Input::postUpdate()
{
    for (int i = 0; i < MAX_BUTTONS; ++i) {
        if (mouseButtons[i] == MouseButtonState::Pressed) {
            mouseButtons[i] = MouseButtonState::Down;
        }
    }
    for (int i = 0; i < MAX_KEYS; ++i) {
        if (keys[i] == KeyState::Pressed) {
            keys[i] = KeyState::Down;
        }
    }

    mousex_prev = mousex;
    mousey_prev = mousey;
}
