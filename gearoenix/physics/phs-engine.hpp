#ifndef GEAROENIX_PHYSICS_ENGINE_HPP
#define GEAROENIX_PHYSICS_ENGINE_HPP
#include "../core/cr-build-configuration.hpp"
#include "../core/cr-types.hpp"
#include <map>
#include <memory>
namespace gearoenix {
namespace core {
    class Semaphore;
}
namespace render {
    class Engine;
}
namespace physics {
    class Kernel;
    namespace animation {
        class Animation;
    }
    class Engine {
        friend class Kernel;

    private:
        render::Engine* render_engine;
        // owner
        // if animation return true on its apply its gonna be deleted
        std::map<core::Id, std::shared_ptr<animation::Animation>> animations;
        bool animations_need_cleaning = false;
#ifdef THREAD_SUPPORTED
        const unsigned int threads_count = 4;
        core::Semaphore* signaller;
        Kernel** kernels;
#else
        Kernel* kernel;
#endif

    protected:
    public:
        Engine(render::Engine* rndeng);
        ~Engine();
        // engine gonna remove it from its active animations, caller must take care of its deleteing
        void add_animation(std::shared_ptr<animation::Animation> a);
        // caller must take care of deleting
        void remove_animation(std::shared_ptr<animation::Animation> a);
        void update();
        void wait();
    };
}
}
#endif
