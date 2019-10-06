#include "rnd-fnt-2d.hpp"
#include "../../core/asset/cr-asset-manager.hpp"
#include "../../core/cr-static.hpp"
#include "../../system/stream/sys-stm-asset.hpp"
#include "../../system/stream/sys-stm-local.hpp"
#include "../../system/stream/sys-stm-memory.hpp"
#include "../../system/sys-app.hpp"
#include "../engine/rnd-eng-engine.hpp"
#include "../texture/rnd-txt-manager.hpp"
#include "../texture/rnd-txt-texture-2d.hpp"
#ifdef GX_IN_WINDOWS
#define STBI_MSC_SECURE_CRT
#endif
#define STBI_WRITE_NO_STDIO
#define STB_TRUETYPE_IMPLEMENTATION
#include <cmath>
#include <stb_truetype.h>
#include <string>

gearoenix::render::font::Font2D::MultilineTextAspectsInfo gearoenix::render::font::Font2D::compute_multiline_text_aspects(const std::wstring& text) const noexcept
{
    MultilineTextAspectsInfo a;
    int w = 1;
    a.height = line_growth + 1;
    const std::size_t txt_size_less = text.size() - 1;
    for (std::size_t i = 0; i < txt_size_less;) {
        const int c = static_cast<int>(text[i]);
        ++i;
        const int next_c = static_cast<int>(text[i]);
        if (c == static_cast<int>('\n')) {
            a.height += line_growth;
            w = 1;
            continue;
        }
        int advance = 0;
        int lsb = 0;
        stbtt_GetCodepointHMetrics(stb_font.get(), c, &advance, &lsb);
        lsb = w - lsb;
        if (a.max_lsb > lsb)
            a.max_lsb = lsb;
        w += advance;
        w += stbtt_GetCodepointKernAdvance(stb_font.get(), c, next_c);
        if (a.max_width < w)
            a.max_width = w;
    }
    {
        const wchar_t c = text[txt_size_less];
        if (c == '\n') {
            a.height += line_growth;
        } else {
            int advance = 0;
            int lsb = 0;
            stbtt_GetCodepointHMetrics(stb_font.get(), c, &advance, &lsb);
            lsb = w - lsb;
            if (a.max_lsb > lsb)
                a.max_lsb = lsb;
            w += advance;
            if (a.max_width < w)
                a.max_width = w;
        }
    }
    a.max_lsb = -a.max_lsb;
    a.max_width += a.max_lsb;
    return a;
}

void gearoenix::render::font::Font2D::init() noexcept
{
    GX_CHECK_NOT_EQAUL_D(0, stbtt_InitFont(stb_font.get(), ttf_data.data(), 0))
    stbtt_GetFontVMetrics(stb_font.get(), &ascent, &descent, &line_gap);
    fnt_height = ascent - descent;
    line_growth = line_gap + fnt_height;
}

gearoenix::render::font::Font2D::Font2D(
    const core::Id my_id,
    system::stream::Stream* f,
    texture::Manager* const txt_mgr) noexcept
    : Font(my_id, Type::D2)
    , txt_mgr(txt_mgr)
    , stb_font(new stbtt_fontinfo())
{
    f->read(ttf_data);
    init();
}

gearoenix::render::font::Font2D::Font2D(const core::Id my_id, texture::Manager* const txt_mgr) noexcept
    : Font(my_id, Type::D2)
    , txt_mgr(txt_mgr)
    , stb_font(new stbtt_fontinfo())
{
    auto* const e = txt_mgr->get_engine();
    std::unique_ptr<system::stream::Asset> asset(system::stream::Asset::construct(e->get_system_application(), "default.ttf"));
    asset->seek(0);
#define DEFAULT_FONT_SIZE 1593268
    ttf_data.resize(DEFAULT_FONT_SIZE);
    asset->read(ttf_data.data(), DEFAULT_FONT_SIZE);
    init();
}

gearoenix::render::font::Font2D::~Font2D() noexcept
{
    stb_font = nullptr;
}

const std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::font::Font2D::multiline_bake(
    const std::wstring& text,
    const std::uint8_t color_bytes[4],
    const int img_width,
    const int img_height,
    const int img_margin,
    core::Real& render_aspect_ratio,
    core::sync::EndCaller<texture::Texture2D> end) const noexcept
{
    const auto a = compute_multiline_text_aspects(text);
    const core::Real w_scale = static_cast<core::Real>(img_width - (img_margin << 1)) / static_cast<core::Real>(a.max_width);
    const core::Real h_scale = static_cast<core::Real>(img_height - (img_margin << 1)) / static_cast<core::Real>(a.height);
    render_aspect_ratio = (w_scale * static_cast<core::Real>(img_margin << 1) + static_cast<core::Real>(a.max_width)) / (h_scale * static_cast<core::Real>(img_margin << 1) + static_cast<core::Real>(a.height));

    core::Real base_line = static_cast<core::Real>(ascent) * h_scale + static_cast<core::Real>(img_margin);
    std::vector<unsigned char> rnd_data(static_cast<std::size_t>(img_width) * static_cast<std::size_t>(img_height));

    const core::Real scaled_lg = static_cast<core::Real>(line_growth) * h_scale;
    const core::Real xpos_start = static_cast<core::Real>(a.max_lsb) * w_scale + static_cast<core::Real>(img_margin);
    const std::size_t txt_size_less = text.size() - 1;
    core::Real xpos = xpos_start;

    const auto index = [&](const int x, const int y) {
        return y * img_width + x;
    };

    core::Real y_shift = base_line - static_cast<core::Real>(floor(base_line));

    for (std::size_t i = 0; i < txt_size_less;) {
        const wchar_t c = text[i];
        ++i;
        const wchar_t next_c = text[i];
        if (c == '\n') {
            base_line += scaled_lg;
            xpos = xpos_start;
            y_shift = base_line - static_cast<core::Real>(floor(base_line));
            continue;
        }
        int advance, lsb, x0, y0, x1, y1;
        const core::Real x_shift = xpos - static_cast<core::Real>(floor(xpos));
        stbtt_GetCodepointHMetrics(stb_font.get(), c, &advance, &lsb);
        stbtt_GetCodepointBitmapBoxSubpixel(stb_font.get(), c, w_scale, h_scale, x_shift, y_shift, &x0, &y0, &x1, &y1);
        stbtt_MakeCodepointBitmapSubpixel(stb_font.get(), &rnd_data[index(int(xpos) + x0, int(base_line) + y0)], x1 - x0, y1 - y0, img_width, w_scale, h_scale, x_shift, y_shift, c);
        xpos += w_scale * (advance + stbtt_GetCodepointKernAdvance(stb_font.get(), c, next_c));
    }
    {
        const wchar_t c = text[txt_size_less];
        if (c != '\n') {
            int x0, y0, x1, y1;
            const core::Real x_shift = xpos - static_cast<core::Real>(floor(xpos));
            stbtt_GetCodepointBitmapBoxSubpixel(stb_font.get(), c, w_scale, h_scale, x_shift, y_shift, &x0, &y0, &x1, &y1);
            stbtt_MakeCodepointBitmapSubpixel(stb_font.get(), &rnd_data[index(int(xpos) + x0, int(base_line) + y0)], x1 - x0, y1 - y0, img_width, w_scale, h_scale, x_shift, y_shift, c);
        }
    }

    const auto pixels_bytes_count = rnd_data.size() << 2;
    unsigned char* img_pixels = new unsigned char[pixels_bytes_count];

    for (std::size_t i = 0, ti = 0, img_index = (static_cast<std::size_t>(img_height) - 1) * static_cast<std::size_t>(img_width) << 2;
         i < static_cast<std::size_t>(img_height);
         ++i, img_index -= (static_cast<std::size_t>(img_width) << 3)) {
        for (std::size_t j = 0; j < static_cast<std::size_t>(img_width); ++j, ++ti) {
            const int c = static_cast<int>(rnd_data[ti]);
            if (c == 0) {
                img_pixels[img_index] = 0;
                ++img_index;
                img_pixels[img_index] = 0;
                ++img_index;
                img_pixels[img_index] = 0;
                ++img_index;
                img_pixels[img_index] = 0;
                ++img_index;
            } else {
                img_pixels[img_index] = color_bytes[0];
                ++img_index;
                img_pixels[img_index] = color_bytes[1];
                ++img_index;
                img_pixels[img_index] = color_bytes[2];
                ++img_index;
                img_pixels[img_index] = static_cast<unsigned char>((static_cast<int>(c) * static_cast<int>(color_bytes[3])) >> 8);
                ++img_index;
            }
        }
    }
    texture::Info txt_info {};
    txt_info.f = texture::TextureFormat::RgbaUint8;
    txt_info.s.mag_filter = texture::Filter::LINEAR;
    txt_info.s.min_filter = texture::Filter::LINEAR;
    txt_info.s.wrap_r = texture::Wrap::CLAMP_TO_EDGE;
    txt_info.s.wrap_s = texture::Wrap::CLAMP_TO_EDGE;
    txt_info.s.wrap_t = texture::Wrap::CLAMP_TO_EDGE;
    txt_info.t = texture::Type::Texture2D;
    core::sync::EndCaller<texture::Texture2D> tend([end](std::shared_ptr<texture::Texture2D> t) mutable noexcept {
        end.set_data(t);
    });
    return txt_mgr->create_2d(img_pixels, txt_info, img_width, img_height, tend);
}
