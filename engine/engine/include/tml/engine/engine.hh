#ifndef ENGINE_ENGINE_ENGINE_HH
#define ENGINE_ENGINE_ENGINE_HH

#include "tml/core/log.hh"
#include "tml/core/uniquePtr.hh"
#include "tml/engine/layer.hh"
#include "tml/platform/IPlatform.hh"
#include "tml/platform/input.hh"
#include "tml/platform/windowDesc.hh"
#include "tml/render/IRenderer.hh"

// Фасад движка и канонический игровой цикл (architecture.md §5.10):
// Fix Your Timestep! — pollEvents → fixedUpdate (0..maxSteps) → render с alpha.

namespace tml
{

struct EngineConfig
{
    WindowDesc window;
    float      fixedDt          = 1.0f / 60.0f;
    unsigned int maxStepsPerFrame = 5;
    LogLevel   logLevel         = LogLevel::Info;
};

class Engine
{
public:
    explicit Engine( const EngineConfig & config = EngineConfig{} )
        : config_( config )
    {
    }

    int run( Layer * layers, std::size_t layerCount );

    rnd::IRenderer & renderer() { return *renderer_; }
    const InputState & input() const { return platform_->input(); }
    const EngineConfig & config() const { return config_; }

private:
    EngineConfig            config_;
    UniquePtr< IPlatform >  platform_;
    UniquePtr< rnd::IRenderer > renderer_;
};

} // namespace tml

#endif // ENGINE_ENGINE_ENGINE_HH
