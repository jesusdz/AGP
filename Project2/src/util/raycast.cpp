#include "raycast.h"
#include "ecs/scene.h"
#include "globals.h"
#include "resources/mesh.h"

#if 0
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
#endif

static bool rayIntersectsPlane(const QVector3D &pos,
                        const QVector3D &dir,
                        const QVector3D &ppos,
                        const QVector3D &pnorm,
                        const QVector3D &boxMin,
                        const QVector3D &boxSize,
                        float *distance)
{
    const float eps = 10.0f * FLT_EPSILON;

    float ldotn = QVector3D::dotProduct(dir, pnorm);
    if (qAbs(ldotn) < FLT_EPSILON) return false; // parallel plane
    float numerator = QVector3D::dotProduct((ppos - pos), pnorm);
    float d = numerator / ldotn;
    if (d < 0.0f) return false; // behind the camera
    *distance = d;
    QVector3D hit = pos + d * dir;
    QVector3D nhit = (hit - boxMin) / boxSize;
    return nhit.x() >= -eps && nhit.x() <= 1.0+eps &&
            nhit.y() >= -eps && nhit.y() <= 1.0+eps &&
            nhit.z() >= -eps && nhit.z() <= 1.0+eps;
}

static bool rayIntersectsBox(const QVector3D &pos, const QVector3D &dir, const Bounds &bounds, float *distance)
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
        if (rayIntersectsPlane(pos, dir, planes[i][0], planes[i][1], bounds.min, boxSize, distance)) {
            return true;
        }
    }
    return false;
}

bool rayCast(const QVector3D &positionWorldspace,
             const QVector3D &directionWorldspace,
             Entity **hit)
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
    }

    return nearestHitDistance < FLT_MAX;
}
