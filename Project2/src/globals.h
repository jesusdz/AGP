#ifndef GLOBALS_H
#define GLOBALS_H

#include "resources/resourcemanager.h"
#include "ecs/scene.h"
#include "ecs/camera.h"
#include "ui/input.h"
#include <QString>

extern ResourceManager *resourceManager;
extern Camera *camera;
extern Scene *scene;
extern Input *input;
extern QString projectDirectory;

#endif // GLOBALS_H
