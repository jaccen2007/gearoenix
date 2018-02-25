#include "rnd-fnt-2d.hpp"
#include "../../system/stream/sys-stm-local.hpp"
#include "../../system/stream/sys-stm-memory.hpp"
#ifdef IN_WINDOWS
#define STBI_MSC_SECURE_CRT
#endif
#define STB_TRUETYPE_IMPLEMENTATION
#include "../../../external/stb/stb_truetype.h"
#ifdef IN_WINDOWS
#define STBI_MSC_SECURE_CRT
#endif
#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../../external/stb/stb_image_write.h"
#include "../rnd-engine.hpp"
#include "../texture/rnd-txt-texture-2d.hpp"
#include <cmath>
#include <string>

#define BAKED_ASPECT 1024
#define FIRST_CHAR 33
#define NUM_CHARS 94

struct PngWriterContext {
    gearoenix::system::stream::Memory* m;
    gearoenix::system::stream::Local* l;
};

static void png_write_function(void* context, void* data, int size)
{
    PngWriterContext* png_ctx = reinterpret_cast<PngWriterContext*>(context);
    png_ctx->m->write(data, size);
    png_ctx->l->write(data, size);
}

gearoenix::render::font::Font2D::Font2D(core::Id my_id, system::stream::Stream* f, Engine* e, core::EndCaller<core::EndCallerIgnore> c)
    : Font(my_id, Shape::D2)
{
    const std::string baked_file_name = std::string("baked_font") + std::to_string(my_id) + ".bin";
    letters_properties.resize(NUM_CHARS);
    const bool baked_exist = system::stream::Local::exist(baked_file_name);
    if (!baked_exist) {
        system::stream::Local baked_file(baked_file_name, true);
        const int root_nchar = (int)std::ceil(std::sqrt((float)NUM_CHARS));
        std::vector<stbtt_bakedchar> bkchar(NUM_CHARS);
        std::vector<unsigned char> ttbuffer;
        f->read(ttbuffer);
        std::vector<unsigned char> atlas_data(BAKED_ASPECT * BAKED_ASPECT);
        stbtt_BakeFontBitmap(
            ttbuffer.data(), 0,
            (float)BAKED_ASPECT / (float)root_nchar,
            &(atlas_data[0]), BAKED_ASPECT, BAKED_ASPECT,
            FIRST_CHAR, NUM_CHARS,
            &(bkchar[0]));
        float yy = 0.0f;
        for (int i = 0; i < NUM_CHARS; ++i) {
            float xx = 0.0f;
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(bkchar.data(), BAKED_ASPECT, BAKED_ASPECT, i, &xx, &yy, &q, 1);
            LetterProperties lp;
            lp.pos_max = math::Vec2(q.x1, q.y1);
            lp.pos_min = math::Vec2(q.x0, q.y0);
            lp.uv_max = math::Vec2(q.s1, q.t1);
            lp.uv_min = math::Vec2(q.s0, q.t0);
            letters_properties[i] = lp;
        }
        baked_file.write(letters_properties.data(), letters_properties.size() * sizeof(LetterProperties));
        system::stream::Memory m;
        core::Count pngsize = 0;
        const core::Count pngsizepos = baked_file.tell();
        m.write(&pngsize, sizeof(pngsize));
        baked_file.write(&pngsize, sizeof(pngsize));
        PngWriterContext pngctx;
        pngctx.m = &m;
        pngctx.l = &baked_file;
        stbi_write_png_to_func(png_write_function, &pngctx, BAKED_ASPECT, BAKED_ASPECT, 1, atlas_data.data(), 0);
        pngsize = m.tell() - sizeof(pngsize);
        m.seek(0);
        baked_file.seek(pngsizepos);
        m.write(&pngsize, sizeof(pngsize));
        baked_file.write(&pngsize, sizeof(pngsize));
        m.seek(0);
        baked_texture = std::shared_ptr<texture::Texture2D>(e->create_texture_2d(&m, core::EndCaller<core::EndCallerIgnore>([c](std::shared_ptr<core::EndCallerIgnore>) -> void {})));
    } else {
        system::stream::Local baked_file(baked_file_name);
        baked_file.read(&(letters_properties[0]), letters_properties.size() * sizeof(LetterProperties));
        baked_texture = std::shared_ptr<texture::Texture2D>(e->create_texture_2d(&baked_file, core::EndCaller<core::EndCallerIgnore>([c](std::shared_ptr<core::EndCallerIgnore>) -> void {})));
    }
}

gearoenix::render::font::Font2D::~Font2D()
{
}

const gearoenix::render::font::Font2D::LetterProperties& gearoenix::render::font::Font2D::get_letter_properties(char c) const
{
    return letters_properties[((unsigned int)c) - FIRST_CHAR];
}

const std::shared_ptr<gearoenix::render::texture::Texture2D> gearoenix::render::font::Font2D::get_baked_texture() const
{
    return baked_texture;
}
