#ifndef GEAROENIX_RENDER_TEXTURE_TEXTURE_2D_HPP
#define GEAROENIX_RENDER_TEXTURE_TEXTURE_2D_HPP
#include "../../core/cr-static.hpp"
#include "rnd-txt-texture.hpp"

namespace gearoenix::render::texture {
class Texture2D : public Texture {
    GX_GET_VAL_PRT(std::size_t, img_width, 0)
    GX_GET_VAL_PRT(std::size_t, img_height, 0)
protected:
    Texture2D(
        const core::Id id,
        const TextureFormat texture_format,
        engine::Engine* const e) noexcept
        : Texture(id, Type::Texture2D, texture_format, e)
    {
    }

public:
    ~Texture2D() noexcept override = default;
};
}
#endif
