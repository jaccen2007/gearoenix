#ifndef GEAROENIX_RENDER_TEXTURE_MANAGER_HPP
#define GEAROENIX_RENDER_TEXTURE_MANAGER_HPP
#include "../../core/cache/cr-cache-file.hpp"
#include "../../core/sync/cr-sync-end-caller.hpp"
#include "../../math/math-vector-4d.hpp"
#include "rnd-txt-texture-info.hpp"
#include <map>
#include <memory>

namespace gearoenix::system::stream {
class Stream;
}

namespace gearoenix::render::engine {
class Engine;
}

namespace gearoenix::render::texture {
class Texture;
class Texture2D;
class TextureCube;
class Manager {
protected:
    engine::Engine* const e;
    core::cache::File<Texture> cache;
    std::map<math::Vec4<float>, core::Id> color_4d_id_t2d;
    std::map<math::Vec3<float>, core::Id> color_3d_id_t2d;
    std::map<math::Vec2<float>, core::Id> color_2d_id_t2d;
    std::map<float, core::Id> color_1d_id_t2d;
    std::map<math::Vec4<float>, core::Id> color_4d_id_cube;
    std::map<math::Vec3<float>, core::Id> color_3d_id_cube;
    std::map<math::Vec2<float>, core::Id> color_2d_id_cube;
    std::shared_ptr<Texture2D> default_one_1c_2d;
    std::shared_ptr<Texture2D> default_one_2c_2d;
    std::shared_ptr<TextureCube> default_zero_3c_cube;
    std::shared_ptr<Texture2D> brdflut;

public:
    Manager(std::unique_ptr<system::stream::Stream> s, engine::Engine* e) noexcept;
    ~Manager() noexcept = default;
    std::shared_ptr<Texture2D> get_2d(const math::Vec4<float>& color, core::sync::EndCaller<Texture2D>& c) noexcept;
    std::shared_ptr<Texture2D> get_2d(const math::Vec3<float>& color, core::sync::EndCaller<Texture2D>& c) noexcept;
    std::shared_ptr<Texture2D> get_2d(const math::Vec2<float>& color, core::sync::EndCaller<Texture2D>& c) noexcept;
    std::shared_ptr<Texture2D> get_2d(float value, core::sync::EndCaller<Texture2D>& c) noexcept;
    std::shared_ptr<Texture2D> get_2d_one_1c(core::sync::EndCaller<Texture2D>& c) noexcept;
    std::shared_ptr<Texture2D> get_2d_one_2c(core::sync::EndCaller<Texture2D>& c) noexcept;
    std::shared_ptr<Texture2D> get_brdflut(core::sync::EndCaller<Texture2D>& c) noexcept;
    /// It will take ownership of data
    std::shared_ptr<Texture2D> create_2d(unsigned char* data, const TextureInfo& info, int img_width, int img_height, core::sync::EndCaller<Texture2D>& c) noexcept;
    /// It creates 2d floating texture from formatted data
    std::shared_ptr<Texture2D> create_2d(const unsigned char* data, std::size_t size, core::sync::EndCaller<Texture2D>& c, const SampleInfo& sample_info = SampleInfo()) noexcept;
    /// It creates 2d floating texture from formatted data for final float images
    std::shared_ptr<Texture2D> create_2d_f(const unsigned char* data, std::size_t size, core::sync::EndCaller<Texture2D>& c, const SampleInfo& sample_info = SampleInfo()) noexcept;
    /// It creates 2d floating texture from file
    std::shared_ptr<Texture2D> create_2d_f(const std::wstring& file_address, core::sync::EndCaller<Texture2D>& c, const SampleInfo& sample_info = SampleInfo()) noexcept;
    std::shared_ptr<Texture2D> create_2d_f(const std::string& file_address, core::sync::EndCaller<Texture2D>& c, const SampleInfo& sample_info = SampleInfo()) noexcept;
    std::shared_ptr<TextureCube> get_cube(const math::Vec4<float>& color, core::sync::EndCaller<TextureCube>& c) noexcept;
    std::shared_ptr<TextureCube> get_cube(const math::Vec3<float>& color, core::sync::EndCaller<TextureCube>& c) noexcept;
    std::shared_ptr<TextureCube> get_cube(const math::Vec2<float>& color, core::sync::EndCaller<TextureCube>& c) noexcept;
    std::shared_ptr<TextureCube> get_cube_zero_3c(core::sync::EndCaller<TextureCube>& c) noexcept;
    std::shared_ptr<TextureCube> create_cube(const TextureInfo& info, int img_aspect, core::sync::EndCaller<TextureCube>& c) noexcept;
    std::shared_ptr<Texture> get_gx3d(core::Id id, core::sync::EndCaller<Texture>& c) noexcept;
    [[nodiscard]] engine::Engine* get_engine() const noexcept;
    [[nodiscard]] static constexpr float geometry_smith(const math::Vec3<float>& n, const math::Vec3<float>& v, const math::Vec3<float>& l, float roughness) noexcept;
    [[nodiscard]] static math::Vec2<float> integrate_brdf(float n_dot_v, float roughness) noexcept;
    [[nodiscard]] static std::vector<math::Vec2<float>> create_brdflut_pixels(std::size_t resolution = GX_DEFAULT_BRDFLUT_RESOLUTION) noexcept;
};
}

constexpr float gearoenix::render::texture::Manager::geometry_smith(
    const math::Vec3<float>& n,
    const math::Vec3<float>& v,
    const math::Vec3<float>& l,
    const float roughness) noexcept
{
    const auto n_dot_v = math::Numeric::maximum(n.dot(v), 0.0f);
    const auto n_dot_l = math::Numeric::maximum(n.dot(l), 0.0f);
    const auto ggx2 = math::Numeric::geometry_schlick_ggx(n_dot_v, roughness);
    const auto ggx1 = math::Numeric::geometry_schlick_ggx(n_dot_l, roughness);
    return ggx1 * ggx2;
}

#endif
