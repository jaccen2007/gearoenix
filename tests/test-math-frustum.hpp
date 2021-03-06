#include <gearoenix/math/math-aabb.hpp>
#include <gearoenix/math/math-frustum.hpp>
#include <gearoenix/math/math-matrix-4d.hpp>
#include <gearoenix/math/math-sphere.hpp>

BOOST_AUTO_TEST_CASE(math_frustum_test)
{
    namespace gxm = gearoenix::math;

    const gxm::Frustum f({
        gxm::Vec3(-10.0, 10.0, -10.0),
        gxm::Vec3(10.0, 10.0, -10.0),
        gxm::Vec3(-10.0, -10.0, -10.0),
        gxm::Vec3(10.0, -10.0, -10.0),
        gxm::Vec3(-1.0, 1.0, 0.0),
        gxm::Vec3(1.0, 1.0, 0.0),
        gxm::Vec3(-1.0, -1.0, 0.0),
        gxm::Vec3(1.0, -1.0, 0.0),
    });

    const gxm::Aabb3 box001(gxm::Vec3(0.0, 0.0, -5.0) + 2.0, gxm::Vec3(0.0, 0.0, -5.0) - 2.0);

    BOOST_CHECK(f.check_intersection_status(box001) == gxm::IntersectionStatus::In);
}