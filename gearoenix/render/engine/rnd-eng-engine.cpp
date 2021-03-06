#include "rnd-eng-engine.hpp"
#include "../../core/asset/cr-asset-manager.hpp"
#include "../../core/cr-function-loader.hpp"
#include "../../core/cr-update-functions-manager.hpp"
#include "../../core/sync/cr-sync-kernel-workers.hpp"
#include "../../core/sync/cr-sync-semaphore.hpp"
#include "../../physics/phs-engine.hpp"
#include "../../system/sys-app.hpp"
#include "../buffer/rnd-buf-manager.hpp"
#include "../command/rnd-cmd-manager.hpp"
#include "../graph/tree/rnd-gr-tr-pbr.hpp"
#include "../pipeline/rnd-pip-manager.hpp"
#include "../sampler/rnd-smp-manager.hpp"
#include "../scene/rnd-scn-manager.hpp"
#include "../scene/rnd-scn-scene.hpp"

void gearoenix::render::engine::Engine::do_late_delete() noexcept
{
    ++late_delete_index;
    late_delete_index %= late_delete_assets.size();
    late_delete_assets[late_delete_index].clear();
}

gearoenix::render::engine::Engine::Engine(system::Application* const system_application, const Type engine_type) noexcept
    : engine_type(engine_type)
    , system_application(system_application)
    , function_loader(new core::FunctionLoader())
    , kernels(new core::sync::KernelWorkers())
    , physics_engine(new physics::Engine(system_application, kernels.get()))
    , update_functions_manager(new core::UpdateFunctionsManager(kernels.get()))
    , late_delete_assets(10)
{
    kernels->add_step(
        [this] {
            function_loader->unload();
            render_tree->update();
            function_loader->unload();
        },
        [this](const unsigned int kernel_index) {
            render_tree->record(kernel_index);
        },
        [] {},
        [this] {
            function_loader->unload();
            render_tree->submit();
        });
}

gearoenix::render::engine::Engine::~Engine() noexcept = default;

void gearoenix::render::engine::Engine::late_delete(std::shared_ptr<core::asset::Asset> asset) noexcept
{
    std::lock_guard<std::mutex> _lg(late_delete_assets_guard);
    late_delete_assets[late_delete_index].push_back(std::move(asset));
}

void gearoenix::render::engine::Engine::update() noexcept
{
    const std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> delta_time_duration = now - last_frame_time;
    delta_time = delta_time_duration.count();
    last_frame_time = now;
    /// TODO: When Vulkan implemented take care of frame_node and do the frame synchronization by it.
    ++frame_number_from_start;
    frame_number = frame_number_from_start % frames_count;
    function_loader->unload();
    physics_engine->update();
    kernels->do_steps();
    do_late_delete();
}

void gearoenix::render::engine::Engine::terminate() noexcept
{
    if (function_loader == nullptr)
        return;
    buffer_manager = nullptr;
    sampler_manager = nullptr;
    command_manager = nullptr;
    pipeline_manager = nullptr;
    physics_engine = nullptr;
    main_render_target = nullptr;
    kernels = nullptr;
    function_loader->unload();
    function_loader = nullptr;
}
