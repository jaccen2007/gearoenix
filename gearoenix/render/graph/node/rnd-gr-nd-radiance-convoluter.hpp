#ifndef GEAROENIX_RENDER_GRAPH_NODE_RADIANCE_CONVOLUTER_HPP
#define GEAROENIX_RENDER_GRAPH_NODE_RADIANCE_CONVOLUTER_HPP
#include "../../../core/cr-pool.hpp"
#include "../../../core/sync/cr-sync-end-caller.hpp"
#include "../../../math/math-matrix-4d.hpp"
#include "../../material/rnd-mat-translucency-mode.hpp"
#include "rnd-gr-nd-node.hpp"
#include <vector>

namespace gearoenix::render::buffer {
class Uniform;
}

namespace gearoenix::render::camera {
class Camera;
}

namespace gearoenix::render::command {
class Buffer;
}

namespace gearoenix::render::mesh {
class Mesh;
}

namespace gearoenix::render::pipeline {
class RadianceConvoluterResourceSet;
}

namespace gearoenix::render::graph::node {

struct RadianceConvoluterUniform {
    float roughness = 0.001f;
    float roughness_p_4 = 0.000000000001f;

    explicit RadianceConvoluterUniform(float r) noexcept;
};

struct RadianceConvoluterKernel {
    const std::unique_ptr<command::Buffer> secondary_cmd;
    const std::unique_ptr<pipeline::RadianceConvoluterResourceSet> r;
    const std::unique_ptr<buffer::Uniform> u;
    RadianceConvoluterKernel(float roughness, engine::Engine* e, pipeline::Pipeline* pip, unsigned int kernel_index) noexcept;
    ~RadianceConvoluterKernel() noexcept;
};

struct RadianceConvoluterFrame {
    std::vector<std::unique_ptr<RadianceConvoluterKernel>> kernels;
    RadianceConvoluterFrame(float roughness, engine::Engine* e, pipeline::Pipeline* pip) noexcept;
    ~RadianceConvoluterFrame() noexcept;
};

class RadianceConvoluter final : public Node {
private:
    std::vector<std::unique_ptr<RadianceConvoluterFrame>> frames;
    RadianceConvoluterFrame* frame = nullptr;
    const mesh::Mesh* const msh;
    const texture::TextureCube* const environment;

    void record(RadianceConvoluterKernel* kernel) noexcept;

public:
    RadianceConvoluter(
        float roughness,
        const mesh::Mesh* msh,
        const texture::TextureCube* environment,
        engine::Engine* e,
        const core::sync::EndCaller<core::sync::EndCallerIgnore>& call) noexcept;
    ~RadianceConvoluter() noexcept final;
    void update() noexcept final;
    void record(unsigned int kernel_index) noexcept final;
    void record_continuously(unsigned int kernel_index) noexcept final;
    void submit() noexcept final;
};
}
#endif
