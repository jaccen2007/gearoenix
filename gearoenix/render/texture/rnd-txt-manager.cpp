#include "rnd-txt-manager.hpp"
#include "../../core/asset/cr-asset-manager.hpp"
#include "../../system/stream/sys-stm-stream.hpp"
#include "../../system/sys-app.hpp"
#include "../engine/rnd-eng-engine.hpp"
#include "rnd-txt-format.hpp"
#include "rnd-txt-png.hpp"
#include "rnd-txt-texture-2d.hpp"
#include "rnd-txt-texture-cube.hpp"
#include "rnd-txt-type.hpp"
#include <array>

gearoenix::render::texture::Manager::Manager(system::stream::Stream* const s, engine::Engine* const e) noexcept
    : e(e)
    , cache(s)
{
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::get_2d(const math::Vec4& color, core::sync::EndCaller<Texture2D>& c) noexcept
{
    static_assert(sizeof(core::Real) == 4, "Only float 32 bit are supported.");
    std::shared_ptr<std::array<core::Real, 4>> cc(new std::array<core::Real, 4>());
    (*cc)[0] = color[0];
    (*cc)[1] = color[1];
    (*cc)[2] = color[2];
    (*cc)[3] = color[3];
    SampleInfo sample_info;
    sample_info.mag_filter = Filter::NEAREST;
    sample_info.min_filter = Filter::NEAREST;
    sample_info.wrap_s = Wrap::REPEAT;
    sample_info.wrap_t = Wrap::REPEAT;
    sample_info.wrap_r = Wrap::REPEAT;
    const auto search = color_4d_id_t2d.find(color);
    const bool found = color_4d_id_t2d.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_4d_id_t2d[color] = id;
    const std::function<std::shared_ptr<Texture>()> fun = [this, cc, c, id, sample_info] {
        return std::shared_ptr<Texture>(e->create_texture_2d(
            id, static_cast<const void*>(cc->data()),
            TextureFormat::RGBA_FLOAT32, sample_info, 1, 1,
            core::sync::EndCaller<core::sync::EndCallerIgnore>([c, cc] {})));
    };
    std::shared_ptr<Texture2D> data = std::static_pointer_cast<Texture2D>(cache.get_cacher().get(id, fun));
    c.set_data(data);
    return data;
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::get_2d(const math::Vec3& color, core::sync::EndCaller<Texture2D>& c) noexcept
{
    const auto search = color_3d_id_t2d.find(color);
    const bool found = color_3d_id_t2d.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_3d_id_t2d[color] = id;
    return get_2d(math::Vec4(color, 1.0f), c);
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::get_2d(const math::Vec2& color, core::sync::EndCaller<Texture2D>& c) noexcept
{
    const auto search = color_2d_id_t2d.find(color);
    const bool found = color_2d_id_t2d.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_2d_id_cube[color] = id;
    return get_2d(math::Vec4(color, 0.0f, 1.0f), c);
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::get_2d(const core::Real value, core::sync::EndCaller<Texture2D>& c) noexcept
{
    const auto search = color_1d_id_t2d.find(value);
    const bool found = color_1d_id_t2d.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_1d_id_t2d[value] = id;
    return get_2d(math::Vec4(value, 0.0f, 0.0f, 1.0f), c);
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::get_2d_one_1c(core::sync::EndCaller<Texture2D>& c) noexcept
{
	if (nullptr == default_one_1c_2d)
		default_one_1c_2d = get_2d(1.0f, c);
	else
		c.set_data(default_one_1c_2d);
	return default_one_1c_2d;
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::get_2d_one_2c(core::sync::EndCaller<Texture2D>& c) noexcept
{
	if (nullptr == default_one_2c_2d)
		default_one_2c_2d = get_2d(1.0f, c);
	else
		c.set_data(default_one_2c_2d);
	return default_one_2c_2d;
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::create_2d(unsigned char *data, const Info& info, int img_width, int img_height, core::sync::EndCaller<Texture2D>& c) noexcept
{
    const auto id = core::asset::Manager::create_id();
    auto t = std::shared_ptr<texture::Texture2D>(e->create_texture_2d(
        id,
        data,
        info.f,
        info.s,
        img_width,
        img_height,
        core::sync::EndCaller<core::sync::EndCallerIgnore>([data, c] { delete data; })));
    c.set_data(t);
    cache.get_cacher().get_cacheds()[id] = t;
    return t;
}

std::shared_ptr<gearoenix::render::texture::Cube> gearoenix::render::texture::Manager::get_cube(const math::Vec4& color, core::sync::EndCaller<Cube>& c) noexcept
{
	/// TODO: It is better to have deferent types of color and elements
    static_assert(sizeof(core::Real) == 4, "Only float 32 bit are supported.");
    std::shared_ptr<std::array<core::Real, 4>> cc(new std::array<core::Real, 4>());
    (*cc)[0] = color[0];
    (*cc)[1] = color[1];
    (*cc)[2] = color[2];
    (*cc)[3] = color[3];
    SampleInfo sample_info;
    sample_info.mag_filter = Filter::NEAREST;
    sample_info.min_filter = Filter::NEAREST;
    sample_info.wrap_s = Wrap::REPEAT;
    sample_info.wrap_t = Wrap::REPEAT;
    sample_info.wrap_r = Wrap::REPEAT;
    const auto search = color_4d_id_cube.find(color);
    const bool found = color_4d_id_cube.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_4d_id_cube[color] = id;
    const std::function<std::shared_ptr<Texture>()> fun = [this, cc, c, id, sample_info] {
        return std::shared_ptr<Texture>(e->create_texture_cube(
            id, static_cast<const void*>(cc->data()),
            TextureFormat::RGBA_FLOAT32, sample_info, 1,
            core::sync::EndCaller<core::sync::EndCallerIgnore>([c, cc] {})));
    };
    std::shared_ptr<Cube> data = std::static_pointer_cast<Cube>(cache.get_cacher().get(id, fun));
    c.set_data(data);
    return data;
}

std::shared_ptr<gearoenix::render::texture::Cube> gearoenix::render::texture::Manager::get_cube(const math::Vec3& color, core::sync::EndCaller<Cube>& c) noexcept
{
    const auto search = color_3d_id_cube.find(color);
    const bool found = color_3d_id_cube.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_3d_id_cube[color] = id;
    return get_cube(math::Vec4(color, 1.0f), c);
}

std::shared_ptr<gearoenix::render::texture::Cube> gearoenix::render::texture::Manager::get_cube(const math::Vec2& color, core::sync::EndCaller<Cube>& c) noexcept
{
    const auto search = color_2d_id_cube.find(color);
    const bool found = color_2d_id_cube.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_2d_id_cube[color] = id;
    return get_cube(math::Vec4(color, 0.0f, 1.0f), c);
}

std::shared_ptr<gearoenix::render::texture::Cube> gearoenix::render::texture::Manager::get_cube_zero_3c(core::sync::EndCaller<Cube>& c) noexcept
{
	if (default_zero_3c_cube == nullptr)
		default_zero_3c_cube = get_cube(math::Vec3(0.0f, 0.0f, 0.0f), c);
	else
		c.set_data(default_zero_3c_cube);
	return default_zero_3c_cube;
}

std::shared_ptr<gearoenix::render::texture::Texture> gearoenix::render::texture::Manager::get_gx3d(const core::Id id, core::sync::EndCaller<Texture>& c) noexcept
{
    const std::shared_ptr<Texture> o = cache.get<Texture>(id, [this, id, c] {
        system::stream::Stream* const f = cache.get_file();
        switch (f->read<Type::Id>()) {
        case Type::TEXTURE_2D: {
            std::shared_ptr<std::vector<unsigned char>> data(new std::vector<unsigned char>);
            unsigned int img_width;
            unsigned int img_height;
            PNG::decode(f, *(data.get()), img_width, img_height);
            core::sync::EndCaller<core::sync::EndCallerIgnore> call([c, data] {});
            SampleInfo sinfo = SampleInfo();
            return std::shared_ptr<Texture>(e->create_texture_2d(
                id, static_cast<const void*>(data->data()), TextureFormat::RGBA_UINT8, sinfo, img_width, img_height, call));
        }
        case Type::TEXTURE_3D:
            GXUNIMPLEMENTED
        case Type::CUBE:
            GXUNIMPLEMENTED
        default:
            GXUNEXPECTED
        }
    });
    c.set_data(o);
    return o;
}

gearoenix::render::engine::Engine* gearoenix::render::texture::Manager::get_engine() const noexcept
{
    return e;
}
