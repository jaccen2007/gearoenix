#include "glc3-pip-radiance-convoluter-resource-set.hpp"
#ifdef GX_USE_OPENGL_CLASS_3
#include "../../gl/gl-loader.hpp"
#include "../../render/buffer/rnd-buf-uniform.hpp"
#include "../../render/camera/rnd-cmr-uniform.hpp"
#include "../../render/graph/node/rnd-gr-nd-radiance-convoluter.hpp"
#include "../../render/mesh/rnd-msh-mesh.hpp"
#include "../buffer/glc3-buf-index.hpp"
#include "../buffer/glc3-buf-vertex.hpp"
#include "../shader/glc3-shd-radiance-convoluter.hpp"
#include "../texture/glc3-txt-cube.hpp"
#include "glc3-pip-radiance-convoluter.hpp"
#include "glc3-pip-resource-set.hpp"

gearoenix::glc3::pipeline::RadianceConvoluterResourceSet::RadianceConvoluterResourceSet(const std::shared_ptr<shader::RadianceConvoluter>& shd, std::shared_ptr<RadianceConvoluter const> pip) noexcept
    : render::pipeline::RadianceConvoluterResourceSet(std::move(pip))
    , base(new glc3::pipeline::ResourceSet(shd))
{
}

gearoenix::glc3::pipeline::RadianceConvoluterResourceSet::~RadianceConvoluterResourceSet() noexcept = default;

void gearoenix::glc3::pipeline::RadianceConvoluterResourceSet::bind_final(gl::uint& bound_shader_program) const noexcept
{
#ifdef GX_DEBUG_GL_CLASS_3
    gl::Loader::check_for_error();
#endif
    GX_GLC3_PIP_RES_START_DRAWING_MESH
    GX_GLC3_PIP_RES_START_SHADER(RadianceConvoluter, shd)
    GX_GLC3_PIP_RES_SET_TXT_CUBE(environment, environment)
    const auto* const node = node_uniform_buffer->get_ptr<render::graph::node::RadianceConvoluterUniform>();
    GX_GLC3_PIP_RES_SET_UNIFORM(roughness, node->roughness)
    GX_GLC3_PIP_RES_SET_UNIFORM(roughness_p_4, node->roughness_p_4)
    GX_GLC3_PIP_RES_END_DRAWING_MESH
#ifdef GX_DEBUG_GL_CLASS_3
    gl::Loader::check_for_error();
#endif
}

#endif