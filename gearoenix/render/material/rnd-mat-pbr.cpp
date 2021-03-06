#include "rnd-mat-pbr.hpp"
#include "../../core/asset/cr-asset-manager.hpp"
#include "../../system/sys-app.hpp"
#include "../buffer/rnd-buf-framed-uniform.hpp"
#include "../texture/rnd-txt-manager.hpp"
#include "../texture/rnd-txt-texture-2d.hpp"
#include "../texture/rnd-txt-texture-cube.hpp"

gearoenix::render::material::Pbr::Pbr(engine::Engine* const e, const core::sync::EndCaller<core::sync::EndCallerIgnore>& end) noexcept
    : Material(Type::Pbr, e, sizeof(Uniform))
    , color_value(math::Vec4(1.0f, 0.0f, 0.0f, 1.0f))
    , emission_value(math::Vec3(0.0f, 0.0f, 0.0f))
    , metallic_roughness_value(math::Vec2(1.0f, 1.0f))
    , normal_value(math::Vec3(0.5f, 0.5f, 1.0f))
{
    core::sync::EndCaller<texture::Texture2D> call_txt_2d([end](const std::shared_ptr<texture::Texture2D>&) {});
    core::sync::EndCaller<texture::TextureCube> call_txt_cube([end](const std::shared_ptr<texture::TextureCube>&) {});
    auto* const txt_mgr = e->get_system_application()->get_asset_manager()->get_texture_manager();
    color_texture = txt_mgr->get_2d(color_value.value(), call_txt_2d);
    emission_texture = txt_mgr->get_2d(emission_value.value(), call_txt_2d);
    metallic_roughness_texture = txt_mgr->get_2d(metallic_roughness_value.value(), call_txt_2d);
    normal_texture = txt_mgr->get_2d(normal_value.value(), call_txt_2d);
    irradiance = radiance = txt_mgr->get_cube_zero_3c(call_txt_cube).get();
}

gearoenix::render::material::Pbr::Pbr(system::stream::Stream* const f, engine::Engine* const e, const core::sync::EndCaller<core::sync::EndCallerIgnore>& end) noexcept
    : Material(Type::Pbr, e, sizeof(Uniform))
{
    auto* const txt_mgr = e->get_system_application()->get_asset_manager()->get_texture_manager();

    // Reading alpha
    if (f->read_bool()) {
        uniform.alpha = 1.0f;
    } else {
        f->read(uniform.alpha);
    }
    // Reading color
    if (f->read_bool()) {
        core::sync::EndCaller<texture::Texture> txt_call([end](const std::shared_ptr<texture::Texture>&) {});
        color_texture = std::dynamic_pointer_cast<texture::Texture2D>(txt_mgr->get_gx3d(f->read<core::Id>(), txt_call));
    } else {
        core::sync::EndCaller<texture::Texture2D> txt_call([end](const std::shared_ptr<texture::Texture2D>&) {});
        math::Vec4<float> color;
        color.read(f);
        color_texture = std::dynamic_pointer_cast<texture::Texture2D>(txt_mgr->get_2d(color, txt_call));
        color_value = color;
    }
    // Reading emission
    if (f->read_bool()) {
        core::sync::EndCaller<texture::Texture> txt_call([end](const std::shared_ptr<texture::Texture>&) {});
        emission_texture = std::dynamic_pointer_cast<texture::Texture2D>(txt_mgr->get_gx3d(f->read<core::Id>(), txt_call));
    } else {
        core::sync::EndCaller<texture::Texture2D> txt_call([end](const std::shared_ptr<texture::Texture2D>&) {});
        math::Vec3<float> emission;
        emission.read(f);
        emission_texture = std::dynamic_pointer_cast<texture::Texture2D>(txt_mgr->get_2d(emission, txt_call));
        emission_value = emission;
    }
    // Reading metallic_roughness
    if (f->read_bool()) {
        core::sync::EndCaller<texture::Texture> txt_call([end](const std::shared_ptr<texture::Texture>&) {});
        metallic_roughness_texture = std::dynamic_pointer_cast<texture::Texture2D>(txt_mgr->get_gx3d(f->read<core::Id>(), txt_call));
    } else {
        core::sync::EndCaller<texture::Texture2D> txt_call([end](const std::shared_ptr<texture::Texture2D>&) {});
        math::Vec2<float> metallic_roughness;
        metallic_roughness.read(f);
        metallic_roughness_texture = std::dynamic_pointer_cast<texture::Texture2D>(txt_mgr->get_2d(metallic_roughness, txt_call));
        metallic_roughness_value = metallic_roughness;
    }
    // Reading normal
    if (f->read_bool()) {
        core::sync::EndCaller<texture::Texture> txt_call([end](const std::shared_ptr<texture::Texture>&) {});
        normal_texture = std::dynamic_pointer_cast<texture::Texture2D>(txt_mgr->get_gx3d(f->read<core::Id>(), txt_call));
    } else {
        core::sync::EndCaller<texture::Texture2D> txt_call([end](const std::shared_ptr<texture::Texture2D>&) {});
        normal_value = math::Vec3(0.5f, 0.5f, 1.0f);
        normal_texture = std::dynamic_pointer_cast<texture::Texture2D>(txt_mgr->get_2d(normal_value.value(), txt_call));
    }
    // Translucency
    if (f->read_bool())
        translucency = TranslucencyMode::Transparent;
    is_shadow_caster = f->read_bool();
    f->read(uniform.alpha_cutoff);
}

gearoenix::render::material::Pbr::~Pbr() noexcept = default;

void gearoenix::render::material::Pbr::Pbr::update() noexcept
{
    uniform_buffers->update(uniform);
}

void gearoenix::render::material::Pbr::set_metallic_factor(const float f) noexcept
{
    uniform.metallic_factor = f;
}

void gearoenix::render::material::Pbr::set_roughness_factor(const float f) noexcept
{
    uniform.roughness_factor = f;
}

void gearoenix::render::material::Pbr::set_color(
    const std::uint32_t code, const core::sync::EndCaller<core::sync::EndCallerIgnore>& end) noexcept
{
    set_color(math::Vec4(
                  float(code >> 24) / 255.0f,
                  float((code >> 16) & 255) / 255.0f,
                  float((code >> 8) & 255) / 255.0f,
                  float(code & 255) / 255.0f),
        end);
}

void gearoenix::render::material::Pbr::set_color(
    const float r, const float g, const float b,
    const core::sync::EndCaller<core::sync::EndCallerIgnore>& end) noexcept
{
    set_color(math::Vec4(r, g, b, 1.0f), end);
}

void gearoenix::render::material::Pbr::set_color(const math::Vec4<float>& c,
    const core::sync::EndCaller<core::sync::EndCallerIgnore>& end) noexcept
{
    core::sync::EndCaller<texture::Texture2D> call_txt_2d([end](const std::shared_ptr<texture::Texture2D>&) {});
    auto* const txt_mgr = e->get_system_application()->get_asset_manager()->get_texture_manager();
    color_texture = txt_mgr->get_2d(c, call_txt_2d);
    color_value = c;
}

void gearoenix::render::material::Pbr::set_color(const std::shared_ptr<texture::Texture2D>& c) noexcept
{
    color_texture = c;
    color_value = std::nullopt;
}

void gearoenix::render::material::Pbr::set_alpha(const float a) noexcept
{
    uniform.alpha = a;
}
