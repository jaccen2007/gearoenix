#ifndef GEAROENIX_GLC3_SHADER_IRRADIANCE_CONVOLUTER_HPP
#define GEAROENIX_GLC3_SHADER_IRRADIANCE_CONVOLUTER_HPP
#include "../../core/cr-build-configuration.hpp"
#ifdef GX_USE_OPENGL_CLASS_3
#include "../../core/cr-static.hpp"
#include "../../core/sync/cr-sync-end-caller.hpp"
#include "../glc3.hpp"
#include "glc3-shd-shader.hpp"
#include <string>

namespace gearoenix::glc3::engine {
class Engine;
}

namespace gearoenix::glc3::shader {
class IrradianceConvoluter final : public Shader {
    GX_GLC3_UNIFORM_TEXTURE(environment)
    GX_GLC3_UNIFORM_MATRIX(mvp, 4, 1)
public:
    IrradianceConvoluter(engine::Engine* e, const core::sync::EndCaller<core::sync::EndCallerIgnore>& c) noexcept;
    ~IrradianceConvoluter() noexcept final;
    void bind() const noexcept final;
};
}

#endif
#endif
