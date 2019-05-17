#include "math-projector-frustum.hpp"
#include "math-aabb.hpp"
#include "math-sphere.hpp"

gearoenix::math::ProjectorFrustum::ProjectorFrustum(const Mat4x4& view_projection) noexcept
    : view_projection(view_projection)
{
}

void gearoenix::math::ProjectorFrustum::set_view_projection(const gearoenix::math::Mat4x4& v) noexcept
{
    view_projection = v;
}

gearoenix::math::IntersectionStatus::Type gearoenix::math::ProjectorFrustum::check_intersection(const Sphere& s) const noexcept
{
    return check_intersection(s.position, s.radius);
}

gearoenix::math::IntersectionStatus::Type gearoenix::math::ProjectorFrustum::check_intersection(const gearoenix::math::Vec3& position, const core::Real radius) const noexcept
{
    return check_intersection({
        position + Vec3(radius, radius, radius),
        position + Vec3(-radius, radius, radius),
        position + Vec3(radius, -radius, radius),
        position + Vec3(-radius, -radius, radius),
        position + Vec3(radius, radius, -radius),
        position + Vec3(-radius, radius, -radius),
        position + Vec3(radius, -radius, -radius),
        position + Vec3(-radius, -radius, -radius),
    });
}

gearoenix::math::IntersectionStatus::Type gearoenix::math::ProjectorFrustum::check_intersection(const Aabb3& aabb) const noexcept
{
    const Vec3 c = (aabb.mx + aabb.mn) * 0.5f;
    const Vec3 r = (aabb.mx - aabb.mn) * 0.5f;
    return check_intersection({
        aabb.mx,
        c + Vec3(-r[0], r[1], r[2]),
        c + Vec3(r[0], -r[1], r[2]),
        c + Vec3(-r[0], -r[1], r[2]),
        c + Vec3(r[0], r[1], -r[2]),
        c + Vec3(-r[0], r[1], -r[2]),
        c + Vec3(r[0], -r[1], -r[2]),
        aabb.mn,
    });
}

gearoenix::math::IntersectionStatus::Type gearoenix::math::ProjectorFrustum::check_intersection(const Vec3 (&points)[8]) const noexcept
{
    Aabb3 aabb;
    for (const Vec3& p : points) {
        const Vec4 pp = view_projection * Vec4(p, 1.0f);
        aabb.put(pp.xyz() / pp[3]);
    }
    static const Aabb3 u(Vec3(1.0f, 1.0f, 1.0f), Vec3(-1.0f, -1.0f, -1.0f));
    return u.check_intersection(aabb);
}
