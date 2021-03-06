#ifndef GEAROENIX_GLES2_PIPELINE_UNLIT_HPP
#define GEAROENIX_GLES2_PIPELINE_UNLIT_HPP
#include "../../core/cr-build-configuration.hpp"
#ifdef GX_USE_OPENGL_ES2
#include "../../render/pipeline/rnd-pip-unlit.hpp"

namespace gearoenix::gles2::engine {
class Engine;
}

namespace gearoenix::gles2::shader {
class Unlit;
}

namespace gearoenix::gles2::pipeline {
class Unlit final : public render::pipeline::Unlit {
private:
    const std::shared_ptr<shader::Unlit> shd;
    std::weak_ptr<Unlit> self;
    Unlit(engine::Engine* e, const core::sync::EndCaller<core::sync::EndCallerIgnore>& c) noexcept;

public:
    [[nodiscard]] static std::shared_ptr<Unlit> construct(engine::Engine* e, const core::sync::EndCaller<core::sync::EndCallerIgnore>& c) noexcept;
    ~Unlit() noexcept final;
    [[nodiscard]] render::pipeline::ResourceSet* create_resource_set() const noexcept final;
};
}

#endif
#endif
