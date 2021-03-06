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
        core::Id id,
        TextureFormat texture_format,
        engine::Engine* e) noexcept;

public:
    ~Texture2D() noexcept override;

    void write_gx3d(
        const std::shared_ptr<system::stream::Stream>& s,
        const core::sync::EndCaller<core::sync::EndCallerIgnore>& c) noexcept override;
};
}
#endif
