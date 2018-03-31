#ifndef GEAROEMIX_RENDER_MODEL_MODEL_HPP
#define GEAROEMIX_RENDER_MODEL_MODEL_HPP
#include "../../core/asset/cr-asset.hpp"
#include "../../core/cr-types.hpp"
#include "../../core/sync/cr-sync-end-caller.hpp"
#include "../../math/math-matrix.hpp"
#include "../../math/math-vector.hpp"
#include "../../physics/phs-transformable.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <tuple>
#include <vector>
namespace gearoenix {
namespace math {
    struct Ray3;
}
namespace physics {
    class Kernel;
    namespace collider {
        class Collider;
    }
}
namespace render {
    class Engine;
    namespace camera {
        class Camera;
    }
    namespace material {
        class Depth;
        class Material;
    }
    namespace mesh {
        class Mesh;
    }
    namespace scene {
        class Scene;
    }
    namespace texture {
        class Texture2D;
    }
    namespace model {
        class Uniform;
        class Model : public core::asset::Asset, public physics::Transferable {
            friend class physics::Kernel;

        public:
            typedef enum : core::Id {
                BASIC = 1,
                WIDGET = 2,
            } ModelType;

        protected:
            const ModelType model_type;
            bool has_shadow_caster = false;
            bool has_transparent = false;
            bool is_in_sun = false;
            bool is_in_camera = false;
            //std::mutex locker;
            bool is_rigid_body;
            bool is_dynamic_rigid_body;
            // be careful about scaling because it gonna scale radius too.
            math::Mat4x4 m;
            bool needs_mvp = false;
            math::Mat4x4 mvp;
            bool needs_dbm = false;
            math::Mat4x4 dbm;
            math::Mat4x4 sunmvp;
            math::Vec3 occloc;
            // be careful about scaling because it gonna scale radius too.
            math::Vec3 occrdss;
            core::Real occrds;
            math::Vec3 moccloc;
            core::Real distcam;
            // mesh_id -> (mesh, material, shadow_caster_material)
            std::map<core::Id,
                std::tuple<std::shared_ptr<mesh::Mesh>,
                    std::shared_ptr<material::Material>,
                    std::shared_ptr<material::Depth>>>
                meshes;
            // model_id -> model
            std::map<core::Id, std::shared_ptr<Model>> children;
            physics::collider::Collider* collider = nullptr;
            std::vector<math::Mat4x4> state;
            Engine* const render_engine = nullptr;
            Model(core::Id my_id, ModelType t, system::stream::Stream* f, Engine* e, core::sync::EndCaller<core::sync::EndCallerIgnore> c);

        public:
            static Model* read(core::Id my_id, system::stream::Stream* f, Engine* e, core::sync::EndCaller<core::sync::EndCallerIgnore> c);
            virtual ~Model();
            ModelType get_type() const;
            void commit(const scene::Scene* s);
            void draw(core::Id mesh_id, texture::Texture2D* shadow_texture);
            void cast_shadow(core::Id mesh_id);
            void push_state();
            void pop_state();
            const std::map<core::Id, std::shared_ptr<Model>>& get_children() const;
            const std::map<core::Id, std::tuple<std::shared_ptr<mesh::Mesh>, std::shared_ptr<material::Material>, std::shared_ptr<material::Depth>>>& get_meshes() const;
            const math::Mat4x4& get_m() const;
            const math::Mat4x4& get_mvp() const;
            const math::Mat4x4& get_sun_mvp() const;
            const physics::collider::Collider* get_collider() const;
            bool hit(const math::Ray3& r, core::Real& distance_from_origin);
            bool get_is_in_camera() const;
            core::Real get_distance_from_camera() const;
            // Transformable---------------------------------------------------------
            void get_location(math::Vec3& l) const;
            void set_location(const math::Vec3& l);
            void translate(const math::Vec3& t);
            void global_rotate(const core::Real d, const math::Vec3& axis);
            void global_rotate(const math::Mat4x4& rm);
            void local_rotate(const core::Real d, const math::Vec3& axis);
            void local_x_rotate(const core::Real d);
            void local_y_rotate(const core::Real d);
            void local_z_rotate(const core::Real d);
            void global_scale(const core::Real s);
            void local_scale(const core::Real s);
        };
    }
}
}
#endif
