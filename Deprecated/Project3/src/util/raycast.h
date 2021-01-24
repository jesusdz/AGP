#ifndef RAYCAST_H
#define RAYCAST_H

class Entity;
class QVector3D;

bool rayCast(const QVector3D &positionWorldspace, const QVector3D &directionWorldspace, Entity **hit);

#endif // RAYCAST
