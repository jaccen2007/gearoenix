#ifndef GEAROENIX_PHYSICS_COLLIDER_HPP
#define GEAROENIX_PHYSICS_COLLIDER_HPP
#include "../../core/cr-types.hpp"
#include <memory>

namespace gearoenix {
namespace math {
    struct Mat4x4;
    struct Ray3;
}
namespace system::stream {
    class Stream;
}
namespace physics::collider {
    class Collider {
    public:
        enum Type : core::Id {
            GHOST = 1,
            MESH = 2,
        };

    protected:
        const Type t;
        explicit Collider(Type t);

    public:
        Collider() = delete;
        virtual ~Collider();
        static Collider* read(const std::shared_ptr<system::stream::Stream>& f);
        virtual void update(const math::Mat4x4& m) = 0;
        virtual bool hit(const math::Ray3& r, core::Real& distance_from_origin) const = 0;
    };
}
}
#endif
