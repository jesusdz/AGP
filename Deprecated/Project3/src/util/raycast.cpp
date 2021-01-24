#include "raycast.h"
#include "ecs/scene.h"
#include "globals.h"
#include "resources/mesh.h"
#include <QtGlobal>
#include <cmath>


/*
static bool rayIntersectsTriangle(const QVector3D &pos,
                                  const QVector3D &dir,
                                  const QVector3D &v0,
                                  const QVector3D &v1,
                                  const QVector3D &v2)
{
    // Determine intersection with plane
    // TODO: Use rayIntersectsPlane

    // Determine if the hit point is inside the triangle
    Vec3f edge0 = v1 - v0;
    Vec3f edge1 = v2 - v1;
    Vec3f edge2 = v0 - v2;
    Vec3f C0 = P - v0;
    Vec3f C1 = P - v1;
    Vec3f C2 = P - v2;
    if (dotProduct(N, crossProduct(edge0, C0)) > 0 &&
    dotProduct(N, crossProduct(edge1, C1)) > 0 &&
    dotProduct(N, crossProduct(edge2, C2)) > 0) return true; // P is inside the triangle
}
*/

static bool rayIntersectsBoxFace(
        const QVector3D &pos,     // Ray origin
        const QVector3D &dir,     // Ray direction
        const QVector3D &ppos,    // Point at plane
        const QVector3D &pnorm,   // Plane normal
        const QVector3D &boxMin,  // Box min vertex
        const QVector3D &boxSize, // Box size
        float *distance)          // Out: hit distance
{
    const float eps = 10.0f * FLT_EPSILON;

    float ldotn = QVector3D::dotProduct(dir, pnorm);
    if (qAbs(ldotn) < FLT_EPSILON) return false; // parallel plane
    float numerator = QVector3D::dotProduct((ppos - pos), pnorm);
    float d = numerator / ldotn;
    if (d < 0.0f) return false; // behind the camera
    *distance = d;
    QVector3D hit = pos + d * dir;

    auto myMax = [](const QVector3D &p, float v) {
        return QVector3D(qMax(p.x(), v), qMax(p.y(), v), qMax(p.z(), v));
    };

    QVector3D nhit = (hit - boxMin) / myMax(boxSize, eps);
    return nhit.x() >= -eps && nhit.x() <= 1.0+eps &&
            nhit.y() >= -eps && nhit.y() <= 1.0+eps &&
            nhit.z() >= -eps && nhit.z() <= 1.0+eps;
}

static bool rayIntersectsSphere(
        const QVector3D &pos,  // Ray origin
        const QVector3D &dir,  // Ray direction
        const QVector3D &spos, // Sphere center
        const float      srad, // Sphere radius
        float *distance)       // Out: hit distance
{
    const float proj = QVector3D::dotProduct(spos - pos, dir);
    const QVector3D lpos = pos + dir * proj;
    const float r = (spos - lpos).length();

    if (r > srad) {
        return false;
    } else {
        float p2 = sqrtf(srad*srad - r*r); // pythagoras
        float p1 = QVector3D::dotProduct(spos - pos, dir);
        *distance = p1 - p2;
        return true;
    }
}

static bool rayIntersectsBox(
        const QVector3D &pos, // Ray origin
        const QVector3D &dir, // Ray direction
        const Bounds &bounds, // Bounding box
        float *distance)      // Out: hit distance
{
    QVector3D planes[6][2] = {
        {bounds.min, QVector3D(-1,  0,  0)},
        {bounds.min, QVector3D( 0, -1,  0)},
        {bounds.min, QVector3D( 0,  0, -1)},
        {bounds.max, QVector3D( 1, 0, 0)},
        {bounds.max, QVector3D( 0, 1, 0)},
        {bounds.max, QVector3D( 0, 0, 1)}
    };

    const QVector3D boxSize = bounds.max - bounds.min;

    for (int i = 0; i < 6; ++i) {
        if (rayIntersectsBoxFace(pos, dir, planes[i][0], planes[i][1], bounds.min, boxSize, distance)) {
            return true;
        }
    }
    return false;
}

bool rayCast(const QVector3D &positionWorldspace,  // Ray origin
             const QVector3D &directionWorldspace, // Ray direction
             Entity **hit)                         // Out: hit entity
{
    *hit = nullptr;

    float nearestHitDistance = FLT_MAX;
    const QVector4D positionWorldspace4 = QVector4D(positionWorldspace, 1.0);
    const QVector4D directionWorldspace4 = QVector4D(directionWorldspace, 0.0);

    for (auto entity : scene->entities)
    {
        auto meshRenderer = entity->meshRenderer;

        if (meshRenderer != nullptr)
        {
            const QMatrix4x4 worldMatrixInv = entity->transform->matrix().inverted();
            const QVector3D posLocal = QVector3D( worldMatrixInv * positionWorldspace4 );
            const QVector3D dirLocal = QVector3D( worldMatrixInv * directionWorldspace4 ).normalized();

            float hitDistance = FLT_MAX;
            if (rayIntersectsBox(posLocal, dirLocal, meshRenderer->mesh->bounds, &hitDistance))
            {
                if (hitDistance < nearestHitDistance)
                {
                    nearestHitDistance = hitDistance;
                    *hit = entity;
                }
            }
        }

        auto lightSource = entity->lightSource;

        if (lightSource != nullptr)
        {
            const QMatrix4x4 worldMatrixInv = entity->transform->matrix().inverted();
            const QVector3D posLocal = QVector3D( worldMatrixInv * positionWorldspace4 );
            const QVector3D dirLocal = QVector3D( worldMatrixInv * directionWorldspace4 ).normalized();
            const QVector3D sphereCenter = QVector3D(0, 0, 0);
            const float sphereRadius = 0.1;

            float hitDistance = FLT_MAX;
            if (rayIntersectsSphere(posLocal, dirLocal, sphereCenter, sphereRadius, &hitDistance))
            {
                if (hitDistance < nearestHitDistance)
                {
                    nearestHitDistance = hitDistance;
                    *hit = entity;
                }
            }
        }
    }

    return nearestHitDistance < FLT_MAX;
}
