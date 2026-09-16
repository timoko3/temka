#include "tml/render/IRenderer.hh"

#include "debugDrawRaylib.hh"
#include "tml/core/log.hh"
#include "tml/platform/IPlatform.hh"

#include <raylib.h>

namespace tml::rnd
{

class RaylibRenderer : public IRenderer
{
public:
    RaylibRenderer()
        : debugDraw_( new DebugDrawRaylib() )
    {
    }

    ~RaylibRenderer() override = default;

    bool init( IPlatform & /*platform*/, const WindowDesc & /*desc*/ ) override
    {
        TM_INFO( "render", "raylib renderer initialized" );
        return true;
    }

    void shutdown() override
    {
    }

    void beginFrame() override
    {
        BeginDrawing();
        ClearBackground( ::Color{ 245, 245, 245, 255 } );
    }

    void endFrame() override
    {
        EndDrawing();
    }

    IDebugDraw & debug() override
    {
        return *debugDraw_;
    }

    const char * backendName() const override
    {
        return "raylib";
    }

private:
    UniquePtr< IDebugDraw > debugDraw_;
};

UniquePtr< IRenderer > createRenderer()
{
    return UniquePtr< IRenderer >( new RaylibRenderer() );
}

} // namespace tml::rnd
