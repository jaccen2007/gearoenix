#include "gles2-pip-forward-pbr-resource-set.hpp"
#ifdef GX_USE_OPENGL_ES2
#include "../../gl/gl-loader.hpp"
#include "../../render/buffer/rnd-buf-uniform.hpp"
#include "../../render/camera/rnd-cmr-uniform.hpp"
#include "../../render/graph/node/rnd-gr-nd-forward-pbr.hpp"
#include "../../render/light/rnd-lt-directional.hpp"
#include "../../render/material/rnd-mat-uniform.hpp"
#include "../../render/mesh/rnd-msh-mesh.hpp"
#include "../../render/model/rnd-mdl-uniform.hpp"
#include "../../render/scene/rnd-scn-uniform.hpp"
#include "../buffer/gles2-buf-index.hpp"
#include "../buffer/gles2-buf-vertex.hpp"
#include "../gles2.hpp"
#include "../shader/gles2-shd-effect-forward-pbr.hpp"
#include "../texture/gles2-txt-2d.hpp"
#include "../texture/gles2-txt-cube.hpp"
#include "../texture/gles2-txt-target.hpp"

gearoenix::gles2::pipeline::ForwardPbrResourceSet::ForwardPbrResourceSet(const std::shared_ptr<shader::ForwardPbr>& shd)
    : gles2::pipeline::ResourceSet(shd)
{
}

void gearoenix::gles2::pipeline::ForwardPbrResourceSet::bind(gl::uint& bound_shader_program) const
{
    GX_GLES2_PIPRES_START_DRAWING_MESH
    GX_GLES2_PIPRES_START_SHADER(ForwardPbr, shd)
    const auto* const camera = camera_uniform_buffer->get_ptr<render::camera::Uniform>();
    GX_GLES2_PIPRES_SET_UNIFORM(camera_position, *(camera->position.data()))
    GX_GLES2_PIPRES_SET_UNIFORM(camera_vp, *(camera->view_projection.data()))
    //static_cast<const texture::Texture2D *>(ambient_occlusion.get())->bind(shdr->get_effect_ambient_occlusion_index());
    reinterpret_cast<const texture::Texture2D*>(brdflut)->bind(static_cast<gl::enumerated>(shdr->get_effect_brdflut_index()));
    reinterpret_cast<const texture::Cube*>(diffuse_environment)->bind(static_cast<gl::enumerated>(shdr->get_effect_diffuse_environment_index()));
    auto directional_lights_shadow_map_indices = shdr->get_effect_shadow_caster_directional_lights_cascades_shadow_map_indices();
    for (int i = 0, tii = 0; i < GX_MAX_DIRECTIONAL_LIGHTS_SHADOW_CASTER; ++i) {
        for (int j = 0; j < GX_MAX_SHADOW_CASCADES; ++j, ++tii) {
            render::texture::Texture2D* t = directional_lights_shadow_maps[i][j];
            if (t == nullptr)
                continue;
            const auto ti = static_cast<gl::enumerated>(directional_lights_shadow_map_indices[tii]);
            if (render::texture::Type::Target2D == t->get_texture_type())
                reinterpret_cast<const texture::Target*>(t)->bind_textures({ ti });
            else
                reinterpret_cast<const texture::Texture2D*>(t)->bind(ti);
        }
    }
    reinterpret_cast<const texture::Cube*>(specular_environment)->bind(static_cast<gl::enumerated>(shdr->get_effect_specular_environment_index()));
    GX_GLES2_PIPRES_BIND_MATERIAL
    const auto* const model = model_uniform_buffer->get_ptr<render::model::Uniform>();
    GX_GLES2_PIPRES_SET_UNIFORM(model_m, *(model->m.data()))
    const auto* const scene = scene_uniform_buffer->get_ptr<render::scene::Uniform>();
    GX_GLES2_PIPRES_SET_UNIFORM(scene_ambient_light, *(scene->ambient_light.data()))
    GX_GLES2_PIPRES_SET_UNIFORM(scene_directional_lights_color, *(scene->directional_lights_color[0].data()))
    GX_GLES2_PIPRES_SET_UNIFORM(scene_directional_lights_direction, *(scene->directional_lights_direction[0].data()))
    GX_GLES2_PIPRES_SET_UNIFORM(scene_directional_lights_count, *(scene->lights_count.data()))
    //shdr->set_scene_ssao_config_data(scene->ssao_config.data());
    const auto* const node = node_uniform_buffer->get_ptr<render::graph::node::ForwardPbrUniform>();
    GX_GLES2_PIPRES_SET_UNIFORM(effect_point_lights_color_min_radius, *(node->point_lights_color_min_radius[0].data()))
    GX_GLES2_PIPRES_SET_UNIFORM(effect_point_lights_count, node->point_lights_count)
    GX_GLES2_PIPRES_SET_UNIFORM(effect_point_lights_position_max_radius, *(node->point_lights_position_max_radius[0].data()))
    GX_GLES2_PIPRES_SET_UNIFORM(effect_shadow_caster_directional_lights_cascades_view_projection_bias, *(node->shadow_caster_directional_lights_cascades_view_projections_bias[0].data()))
    GX_GLES2_PIPRES_SET_UNIFORM(effect_shadow_caster_directional_lights_color_cascades_count, *(node->shadow_caster_directional_lights_color_cascades_count[0].data()))
    GX_GLES2_PIPRES_SET_UNIFORM(effect_shadow_caster_directional_lights_count, node->shadow_caster_directional_lights_count)
    GX_GLES2_PIPRES_SET_UNIFORM(effect_shadow_caster_directional_lights_direction, *(node->shadow_caster_directional_lights_direction[0].data()))
    GX_GLES2_PIPRES_END_DRAWING_MESH
}
#endif