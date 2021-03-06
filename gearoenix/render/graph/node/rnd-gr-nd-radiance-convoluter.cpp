#include "rnd-gr-nd-radiance-convoluter.hpp"
#include "../../../core/asset/cr-asset-manager.hpp"
#include "../../../core/sync/cr-sync-kernel-workers.hpp"
#include "../../../physics/collider/phs-cld-collider.hpp"
#include "../../../system/sys-app.hpp"
#include "../../buffer/rnd-buf-manager.hpp"
#include "../../buffer/rnd-buf-uniform.hpp"
#include "../../camera/rnd-cmr-camera.hpp"
#include "../../command/rnd-cmd-buffer.hpp"
#include "../../command/rnd-cmd-manager.hpp"
#include "../../engine/rnd-eng-engine.hpp"
#include "../../mesh/rnd-msh-mesh.hpp"
#include "../../pipeline/rnd-pip-manager.hpp"
#include "../../pipeline/rnd-pip-pipeline.hpp"
#include "../../pipeline/rnd-pip-radiance-convoluter-resource-set.hpp"
#include "../../sync/rnd-sy-semaphore.hpp"
#include "../../texture/rnd-txt-manager.hpp"
#include "../../texture/rnd-txt-target.hpp"
#include "../../texture/rnd-txt-texture-cube.hpp"

gearoenix::render::graph::node::RadianceConvoluterUniform::RadianceConvoluterUniform(float r) noexcept
    : roughness(r)
    , roughness_p_4(r * r * r * r)
{
}

gearoenix::render::graph::node::RadianceConvoluterKernel::RadianceConvoluterKernel(
    const float roughness,
    engine::Engine* const e,
    pipeline::Pipeline* const pip,
    const unsigned int kernel_index) noexcept
    : secondary_cmd(e->get_command_manager()->create_secondary_command_buffer(kernel_index))
    , r(dynamic_cast<pipeline::RadianceConvoluterResourceSet*>(pip->create_resource_set()))
    , u(e->get_buffer_manager()->create_uniform(sizeof(RadianceConvoluterUniform)))
{
    RadianceConvoluterUniform ud(roughness);
    u->set_data(ud);
    r->set_node_uniform_buffer(u.get());
}

gearoenix::render::graph::node::RadianceConvoluterKernel::~RadianceConvoluterKernel() noexcept = default;

gearoenix::render::graph::node::RadianceConvoluterFrame::RadianceConvoluterFrame(
    const float roughness,
    engine::Engine* const e,
    pipeline::Pipeline* const pip) noexcept
    : kernels(e->get_kernels()->get_threads_count())
{
    for (std::size_t i = 0; i < kernels.size(); ++i) {
        kernels[i] = std::make_unique<RadianceConvoluterKernel>(roughness, e, pip, static_cast<unsigned int>(i));
    }
}

gearoenix::render::graph::node::RadianceConvoluterFrame::~RadianceConvoluterFrame() noexcept = default;

void gearoenix::render::graph::node::RadianceConvoluter::record(RadianceConvoluterKernel* const kernel) noexcept
{
    auto* const prs = kernel->r.get();
    prs->set_mesh(msh);
    prs->set_environment(environment);
    kernel->secondary_cmd->bind(prs);
}

gearoenix::render::graph::node::RadianceConvoluter::RadianceConvoluter(
    const float roughness,
    const mesh::Mesh* const msh,
    const texture::TextureCube* const environment,
    engine::Engine* const e,
    const core::sync::EndCaller<core::sync::EndCallerIgnore>& call) noexcept
    : Node(
        Type::RadianceConvoluter,
        e,
        pipeline::Type::RadianceConvoluter,
        0,
        1,
        {},
        {
            "color",
        },
        call)
    , frames(e->get_frames_count())
    , msh(msh)
    , environment(environment)
{
    set_providers_count(input_textures.size());
    for (auto& f : frames) {
        f = std::make_unique<RadianceConvoluterFrame>(roughness, e, render_pipeline.get());
    }
}

gearoenix::render::graph::node::RadianceConvoluter::~RadianceConvoluter() noexcept = default;

void gearoenix::render::graph::node::RadianceConvoluter::update() noexcept
{
    Node::update();
    const auto frame_number = e->get_frame_number();
    frame = frames[frame_number].get();
    auto& kernels = frame->kernels;
    for (auto& kernel : kernels) {
        kernel->secondary_cmd->begin();
    }
}

void gearoenix::render::graph::node::RadianceConvoluter::record(const unsigned int) noexcept
{
    GXUNEXPECTED
}

void gearoenix::render::graph::node::RadianceConvoluter::record_continuously(const unsigned int kernel_index) noexcept
{
    auto* const kernel = frame->kernels[kernel_index].get();
    record(kernel);
}

void gearoenix::render::graph::node::RadianceConvoluter::submit() noexcept
{
    const unsigned int frame_number = e->get_frame_number();
    command::Buffer* cmd = frames_primary_cmd[frame_number].get();
    cmd->bind(render_target);
    for (const auto& k : frame->kernels) {
        cmd->record(k->secondary_cmd.get());
    }
    Node::submit();
}
