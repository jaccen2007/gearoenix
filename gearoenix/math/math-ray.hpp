#ifndef GEAROENIX_MATH_RAY_HPP
#define GEAROENIX_MATH_RAY_HPP
#include "math-vector.hpp"
#include "../core/cr-static.hpp"
#include <utility>
namespace gearoenix::math {
struct Ray3 {
    GX_GET_REF_PRV(Vec3, origin)
	GX_GET_REF_PRV(Vec3, normalized_direction)
public:
	Ray3(Vec3 o, Vec3 nd) noexcept: origin(std::move(o)), normalized_direction(std::move(nd)) {}
};
}
#endif