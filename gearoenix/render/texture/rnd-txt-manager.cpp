#include "rnd-txt-manager.hpp"
#include "../../core/asset/cr-asset-manager.hpp"
#include "../../system/stream/sys-stm-asset.hpp"
#include "../../system/stream/sys-stm-local.hpp"
#include "../../system/sys-app.hpp"
#include "../engine/rnd-eng-engine.hpp"
#include "rnd-txt-image.hpp"
#include "rnd-txt-texture-2d.hpp"
#include "rnd-txt-texture-cube.hpp"
#include <array>
#ifndef GX_THREAD_NOT_SUPPORTED
#include <thread>
#endif
//#include <algorithm>
//#include <execution>

gearoenix::render::texture::Manager::Manager(std::unique_ptr<system::stream::Stream> s, engine::Engine* const e) noexcept
    : e(e)
    , cache(std::move(s))
{
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::get_2d(const math::Vec4<float>& color, core::sync::EndCaller<Texture2D>& c) noexcept
{
    static_assert(sizeof(float) == 4, "Only float 32 bit are supported.");
    const auto search = color_4d_id_t2d.find(color);
    const bool found = color_4d_id_t2d.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_4d_id_t2d[color] = id;
    const std::function<std::shared_ptr<Texture>()> fun = [this, color, c, id] {
        return e->create_texture_2d(
            id, color.data(),
            TextureInfo {
                .format = TextureFormat::RgbaFloat32,
                .sample_info = SampleInfo {
                    .min_filter = Filter::Nearest,
                    .mag_filter = Filter::Nearest,
                    .wrap_s = Wrap::Repeat,
                    .wrap_t = Wrap::Repeat,
                    .wrap_r = Wrap::Repeat,
                },
                .texture_type = Type::Texture2D,
                .has_mipmap = false,
            },
            1, 1, core::sync::EndCaller<core::sync::EndCallerIgnore>([c] {}));
    };
    std::shared_ptr<Texture2D> data = std::dynamic_pointer_cast<Texture2D>(cache.get_cacher().get(id, fun));
    c.set_data(data);
    return data;
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::get_2d(const math::Vec3<float>& color, core::sync::EndCaller<Texture2D>& c) noexcept
{
    const auto search = color_3d_id_t2d.find(color);
    const bool found = color_3d_id_t2d.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_3d_id_t2d[color] = id;
    return get_2d(math::Vec4(color, 1.0f), c);
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::get_2d(const math::Vec2<float>& color, core::sync::EndCaller<Texture2D>& c) noexcept
{
    const auto search = color_2d_id_t2d.find(color);
    const bool found = color_2d_id_t2d.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_2d_id_cube[color] = id;
    return get_2d(math::Vec4(color, 0.0f, 1.0f), c);
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::get_2d(const float value, core::sync::EndCaller<Texture2D>& c) noexcept
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

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::get_brdflut(core::sync::EndCaller<Texture2D>& c) noexcept
{
    const auto resolution = e->get_system_application()->get_configuration().render_config.get_brdflut_resolution();
    if (nullptr != brdflut)
        return brdflut;
    {
        const std::unique_ptr<system::stream::Asset> brdf_asset(
            system::stream::Asset::construct(e->get_system_application(), "default-brdflut.png"));
        if (brdf_asset != nullptr) {
            GXLOGD("BRDFLUT asset has been found.")
            const auto data = brdf_asset->get_file_content();
            brdflut = create_2d(data.data(), data.size(), c);
            return brdflut;
        }
    }
    const auto baked_path = std::string("baked-brdflut-x") + std::to_string(resolution) + ".png";
    {
        const std::unique_ptr<system::stream::Local> brdf_cached(system::stream::Local::open(baked_path));
        if (brdf_cached != nullptr) {
            GXLOGD("BRDFLUT baked file has been found.")
            const auto data = brdf_cached->get_file_content();
            brdflut = create_2d(data.data(), data.size(), c);
            return brdflut;
        }
    }
    const auto pixels = create_brdflut_pixels();
    {
        std::vector<unsigned char> data(resolution * resolution * 4);
        std::size_t data_index = 0;
        for (const auto& p : pixels) {
            {
                const auto b = static_cast<int>(p.x * 255.0f);
                if (b < 0) {
                    data[data_index] = 0;
                } else if (b > 255) {
                    data[data_index] = 255;
                } else {
                    data[data_index] = static_cast<unsigned char>(b);
                }
            }
            ++data_index;
            {
                const auto b = static_cast<int>(p.y * 255.0f);
                if (b < 0) {
                    data[data_index] = 0;
                } else if (b > 255) {
                    data[data_index] = 255;
                } else {
                    data[data_index] = static_cast<unsigned char>(b);
                }
            }
            data[++data_index] = 0;
            data[++data_index] = 255;
            ++data_index;
        }
        const std::unique_ptr<system::stream::Local> brdf_baked(new system::stream::Local(baked_path, true));
        Image::encode_png(brdf_baked.get(), data.data(), resolution, resolution, 4);
    }
    const auto data_size = pixels.size() * sizeof(math::Vec2<float>);
    auto* const data = new unsigned char[data_size];
    std::memcpy(data, pixels.data(), data_size);
    brdflut = create_2d(
        data,
        TextureInfo {
            .format = TextureFormat::RgFloat32,
            .sample_info = SampleInfo {
                .wrap_s = Wrap::ClampToEdge,
                .wrap_t = Wrap::ClampToEdge,
                .wrap_r = Wrap::ClampToEdge,
            },
            .texture_type = Type::Texture2D,
            .has_mipmap = true },
        resolution, resolution, c);
    return brdflut;
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::create_2d(unsigned char* const data, const TextureInfo& info, int img_width, int img_height, core::sync::EndCaller<Texture2D>& c) noexcept
{
    const auto id = core::asset::Manager::create_id();
    auto t = e->create_texture_2d(
        id,
        data,
        info,
        img_width,
        img_height,
        core::sync::EndCaller<core::sync::EndCallerIgnore>([data, c] { delete[] data; }));
    c.set_data(t);
    cache.get_cacher().get_cacheds()[id] = t;
    return t;
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::create_2d(
    const unsigned char* const data,
    const std::size_t size,
    core::sync::EndCaller<Texture2D>& c,
    const SampleInfo& sample_info) noexcept
{
    std::size_t img_width;
    std::size_t img_height;
    std::size_t img_channels;
    std::vector<unsigned char> pixels;
    Image::decode(data, size, std::nullopt, pixels, img_width, img_height, img_channels);
    GXLOGD("Texture 2D Image imported with file size: " << size << ", width: " << img_width << " height: " << img_height << ", channels: " << img_channels)
    TextureInfo info;
    info.sample_info = sample_info;
    switch (img_channels) {
    case 1:
        info.format = TextureFormat::Uint8;
        break;
    case 2:
        info.format = TextureFormat::RgUint8;
        break;
    case 3:
        info.format = TextureFormat::RgbUint8;
        break;
    case 4:
        info.format = TextureFormat::RgbaUint8;
        break;
    default:
        GXUNEXPECTED
    }
    info.texture_type = Type::Texture2D;
    const auto id = core::asset::Manager::create_id();
    auto t = e->create_texture_2d(
        id,
        pixels.data(),
        info,
        static_cast<unsigned int>(img_width),
        static_cast<unsigned int>(img_height),
        core::sync::EndCaller<core::sync::EndCallerIgnore>([c] {}));
    c.set_data(t);
    cache.get_cacher().get_cacheds()[id] = t;
    return t;
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::create_2d_f(
    const unsigned char* const data,
    const std::size_t size, core::sync::EndCaller<Texture2D>& c,
    const SampleInfo& sample_info) noexcept
{
    std::size_t img_width;
    std::size_t img_height;
    std::size_t img_channels;
    std::vector<float> pixels;
    Image::decode(data, size, std::nullopt, pixels, img_width, img_height, img_channels);
    GXLOGD("Texture 2D Image imported with file size: " << size << ", width: " << img_width << " height: " << img_height << ", channels: " << img_channels)
    TextureInfo info;
    info.sample_info = sample_info;
    switch (img_channels) {
    case 1:
        GXUNIMPLEMENTED
    case 2:
        GXUNIMPLEMENTED
    case 3:
        info.format = TextureFormat::RgbFloat32;
        break;
    case 4:
        GXUNIMPLEMENTED
    default:
        GXUNEXPECTED
    }
    info.texture_type = Type::Texture2D;
    const auto id = core::asset::Manager::create_id();
    auto t = e->create_texture_2d(
        id,
        pixels.data(),
        info,
        static_cast<unsigned int>(img_width),
        static_cast<unsigned int>(img_height),
        core::sync::EndCaller<core::sync::EndCallerIgnore>([c] {}));
    c.set_data(t);
    cache.get_cacher().get_cacheds()[id] = t;
    return t;
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::create_2d_f(
    const std::wstring& file_address,
    core::sync::EndCaller<Texture2D>& c,
    const SampleInfo& sample_info) noexcept
{
    auto const file_content = system::stream::Stream::get_file_content(file_address);
    return create_2d_f(reinterpret_cast<const unsigned char*>(file_content.data()), file_content.size(), c, sample_info);
}

std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::texture::Manager::create_2d_f(
    const std::string& file_address,
    core::sync::EndCaller<render::texture::Texture2D>& c,
    const SampleInfo& sample_info) noexcept
{
    auto const file_content = system::stream::Stream::get_file_content(file_address);
    return create_2d_f(reinterpret_cast<const unsigned char*>(file_content.data()), file_content.size(), c, sample_info);
}

std::shared_ptr<gearoenix::render::texture::TextureCube> gearoenix::render::texture::Manager::get_cube(const math::Vec4<float>& color, core::sync::EndCaller<TextureCube>& c) noexcept
{
    /// TODO: It is better to have different types of color and elements
    static_assert(sizeof(float) == 4, "Only float 32 bit are supported.");
    const auto search = color_4d_id_cube.find(color);
    const bool found = color_4d_id_cube.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_4d_id_cube[color] = id;
    const std::function<std::shared_ptr<Texture>()> fun = [this, color, c, id] {
        const math::Vec4<float> colors[6] = { color, color, color, color, color, color };
        return e->create_texture_cube(
            id, colors[0].data(),
            TextureInfo {
                .format = TextureFormat::RgbaFloat32,
                .sample_info = SampleInfo {
                    .min_filter = Filter::Nearest,
                    .mag_filter = Filter::Nearest,
                    .wrap_s = Wrap::Repeat,
                    .wrap_t = Wrap::Repeat,
                    .wrap_r = Wrap::Repeat,
                },
                .texture_type = Type::TextureCube,
                .has_mipmap = false,
            },
            1, core::sync::EndCaller<core::sync::EndCallerIgnore>([c] {}));
    };
    std::shared_ptr<TextureCube> data = std::dynamic_pointer_cast<TextureCube>(cache.get_cacher().get(id, fun));
    c.set_data(data);
    return data;
}

std::shared_ptr<gearoenix::render::texture::TextureCube> gearoenix::render::texture::Manager::get_cube(const math::Vec3<float>& color, core::sync::EndCaller<TextureCube>& c) noexcept
{
    const auto search = color_3d_id_cube.find(color);
    const bool found = color_3d_id_cube.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_3d_id_cube[color] = id;
    return get_cube(math::Vec4(color, 1.0f), c);
}

std::shared_ptr<gearoenix::render::texture::TextureCube> gearoenix::render::texture::Manager::get_cube(const math::Vec2<float>& color, core::sync::EndCaller<TextureCube>& c) noexcept
{
    const auto search = color_2d_id_cube.find(color);
    const bool found = color_2d_id_cube.end() != search;
    const core::Id id = found ? search->second : core::asset::Manager::create_id();
    if (!found)
        color_2d_id_cube[color] = id;
    return get_cube(math::Vec4(color, 0.0f, 1.0f), c);
}

std::shared_ptr<gearoenix::render::texture::TextureCube> gearoenix::render::texture::Manager::get_cube_zero_3c(core::sync::EndCaller<TextureCube>& c) noexcept
{
    if (default_zero_3c_cube == nullptr)
        default_zero_3c_cube = get_cube(math::Vec3(0.0f, 0.0f, 0.0f), c);
    else
        c.set_data(default_zero_3c_cube);
    return default_zero_3c_cube;
}

std::shared_ptr<gearoenix::render::texture::TextureCube> gearoenix::render::texture::Manager::create_cube(
    const TextureInfo& info, const int img_aspect, core::sync::EndCaller<TextureCube>& c) noexcept
{
    const auto id = core::asset::Manager::create_id();
    const std::function<std::shared_ptr<Texture>()> fun = [this, &info, img_aspect, &c, id] {
        return e->create_texture_cube(id, nullptr, info, img_aspect, core::sync::EndCaller<core::sync::EndCallerIgnore>([c] {}));
    };
    const auto data = std::dynamic_pointer_cast<TextureCube>(cache.get_cacher().get(id, fun));
    c.set_data(data);
    return data;
}

std::shared_ptr<gearoenix::render::texture::Texture> gearoenix::render::texture::Manager::get_gx3d(const core::Id id, core::sync::EndCaller<Texture>& c) noexcept
{
    const std::shared_ptr<Texture> o = cache.get<Texture>(id, [this, id, c] {
        system::stream::Stream* const f = cache.get_file();
        core::sync::EndCaller<core::sync::EndCallerIgnore> call([c] {});
        switch (f->read<Type>()) {
        case Type::Texture2D: {
            std::vector<unsigned char> data;
            std::size_t img_width;
            std::size_t img_height;
            Image::decode(f, data, img_width, img_height);
            return e->create_texture_2d(
                id, data.data(), TextureInfo {
                                     .format = TextureFormat::RgbaUint8,
                                     .texture_type = Type::Texture2D,
                                 },
                img_width, img_height, call);
        }
        case Type::Texture3D:
            GXUNIMPLEMENTED
        case Type::TextureCube:
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

gearoenix::math::Vec2<float> gearoenix::render::texture::Manager::integrate_brdf(const float n_dot_v, const float roughness) noexcept
{
    const math::Vec3<float> v(std::sqrt(1.0f - n_dot_v * n_dot_v), 0.0f, n_dot_v);

    float a = 0.0f;
    float b = 0.0f;

    const auto n = math::Vec3<float>(0.0f, 0.0f, 1.0f);

    const unsigned int SAMPLE_COUNT = 1024u;
    for (unsigned int i = 0u; i < SAMPLE_COUNT; ++i) {
        // generates a sample vector that's biased towards the
        // preferred alignment direction (importance sampling).
        const auto xi = math::Vec2<float>::hammersley(i, SAMPLE_COUNT);
        const auto h = math::Vec3<float>::importance_sample_ggx(xi, n, roughness);
        const auto l = (h * (2.0f * v.dot(h)) - v).normalized();

        const auto n_dot_l = math::Numeric::maximum(l.z, 0.0f);
        const auto n_dot_h = math::Numeric::maximum(h.z, 0.0f);
        const auto v_dot_h = math::Numeric::maximum(v.dot(h), 0.0f);

        if (n_dot_l > 0.0f) {
            const auto g = geometry_smith(n, v, l, roughness);
            const auto g_vis = (g * v_dot_h) / (n_dot_h * n_dot_v);
            auto tmp = 1.0f - v_dot_h;
            auto tmp_p2 = tmp * tmp;
            const auto fc = tmp * tmp_p2 * tmp_p2;
            tmp = fc * g_vis;
            a += g_vis - tmp;
            b += tmp;
        }
    }
    a /= float(SAMPLE_COUNT);
    b /= float(SAMPLE_COUNT);
    return math::Vec2<float>(a, b);
}

std::vector<gearoenix::math::Vec2<float>> gearoenix::render::texture::Manager::create_brdflut_pixels(const std::size_t resolution) noexcept
{
    std::vector<math::Vec2<float>> pixels(resolution * resolution);
    const auto inv_res = 1.0f / float(resolution);
    // Because the current support of parallel for is incomplete in important compilers, following lines commented out
    //    std::for_each(std::execution::par, pixels.begin(), pixels.end(), [&](math::Vec2<float> &p) {
    //        const auto i = (reinterpret_cast<std::size_t>(&p) -
    //                reinterpret_cast<std::size_t>(&(pixels[0]))) / sizeof(decltype(pixels[0]));
    //        const auto r = i / resolution;
    //        const auto c = i - r * resolution;
    //        p = integrate_brdf(float(c) * inv_res, float(r) * inv_res);
    //    });
#ifdef GX_THREAD_NOT_SUPPORTED
    for (std::size_t r = 0, i = 0; r < resolution; ++r) {
        for (std::size_t c = 0; c < resolution; ++c, ++i) {
            pixels[i] = integrate_brdf((float(c) + 0.5f) * inv_res, (float(r) + 0.5f) * inv_res);
        }
    }
#else
    std::vector<std::thread> threads;
    const auto threads_count = std::thread::hardware_concurrency();
    const auto rows_per_thread = resolution / threads_count;
    for (unsigned int thread_index = 0, starting_row = 0; thread_index < threads_count; ++thread_index) {
        const auto ending_row = starting_row + rows_per_thread;
        threads.emplace_back([&, starting_row { starting_row }, ending_row { ending_row < resolution ? ending_row : resolution }] {
            for (unsigned int r = starting_row, i = starting_row * resolution; r < ending_row; ++r) {
                for (std::size_t c = 0; c < resolution; ++c, ++i) {
                    pixels[i] = integrate_brdf((float(c) + 0.5f) * inv_res, (float(r) + 0.5f) * inv_res);
                }
            }
        });
        starting_row = ending_row;
    }
    for (auto& t : threads) {
        t.join();
    }
#endif
    return pixels;
}
