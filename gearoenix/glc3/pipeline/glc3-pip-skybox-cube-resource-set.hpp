#ifndef GEAROENIX_GLC3_PIPELINE_UNLIT_RESOURCE_SET_HPP
#define GEAROENIX_GLC3_PIPELINE_UNLIT_RESOURCE_SET_HPP
#include "../../core/cr-build-configuration.hpp"
#ifdef GX_USE_OPENGL_CLASS_3
#include "../../core/sync/cr-sync-end-caller.hpp"
#include "../../render/pipeline/rnd-pip-unlit-resource-set.hpp"
#include "glc3-pip-resource-set.hpp"

namespace gearoenix::glc3::shader {
class Unlit;
}

namespace gearoenix::glc3::pipeline {
class Unlit;
class UnlitResourceSet : public ResourceSet, public render::pipeline::UnlitResourceSet {
public:
    UnlitResourceSet(const std::shared_ptr<shader::Unlit>& shd, std::shared_ptr<Unlit const> pip) noexcept;
    ~UnlitResourceSet() noexcept final;
    void bind_final(gl::uint& bound_shader_program) const noexcept;
};
}

#endif
#endif
