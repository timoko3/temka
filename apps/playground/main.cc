#include "tml/engine/engine.hh"
#include "tml/engine/layer.hh"

#include "tml/physics/particleWorld.hh"
#include "tml/physics/world2d.hh"
#include "tml/render/IDebugDraw.hh"
#include "tml/render/IRenderer.hh"
#include "tml/render/camera.hh"
#include "tml/render/color.hh"
#include "tml/math/vec2.hh"
#include "tml/math/vec3.hh"

#include <vector>

using namespace tml;
using namespace tml::math;
using namespace tml::rnd;
using namespace tml::phys;

namespace
{

class ParticleDemoLayer : public Layer
{
public:
    void onAttach( Engine & /*engine*/ ) override
    {
        world_ = new ParticleWorld();
        world_->setGravity( Vec2{ 0.0f, -9.81f } );

        // Детерминированный «фонтан» из 30 частиц.
        for ( int i = 0; i < 30; ++i )
        {
            const float t = static_cast< float >( i - 14 );

            BodyDesc2D desc;
            desc.position    = Vec2{ t * 0.7f, 18.0f + ( i % 5 ) * 1.5f };
            desc.velocity    = Vec2{ t * 0.15f, 0.0f };
            desc.mass        = 1.0f;
            desc.restitution = 0.7f;
            desc.radius      = 0.4f;
            particles_.push_back( world_->createBody( desc ) );
        }

        // Камера: центр (0, 8), половина высоты 14, aspect 16:9
        camera_ = Camera::ortho2D( Vec2{ 0.0f, 8.0f }, 14.0f, 16.0f / 9.0f );
    }

    void onDetach() override
    {
        delete world_;
        world_ = nullptr;
    }

    void onFixedUpdate( float fixedDt ) override
    {
        world_->step( fixedDt );
    }

    void onRender( IRenderer & renderer ) override
    {
        IDebugDraw & debug = renderer.debug();

        debug.begin( camera_ );

        // Пол.
        debug.line( Vec3{ -15.0f, -5.0f, 0.0f }, Vec3{ 15.0f, -5.0f, 0.0f }, Black() );

        // Частицы.
        world_->debugDraw( debug );

        // Подпись.
        debug.text( Vec3{ -18.0f, 20.0f, 0.0f },
                    "temka playground (stub): close window to exit",
                    Black() );

        debug.end();
    }

private:
    ParticleWorld * world_        = nullptr;
    Camera          camera_;
    std::vector< BodyId > particles_;
};

} // anonymous namespace

int main()
{
    EngineConfig cfg;
    cfg.window.title = "temka playground";
    cfg.window.width = 1280;
    cfg.window.height = 720;
    cfg.window.resizable = true;
    cfg.window.vsync = true;

    Engine engine( cfg );

    ParticleDemoLayer layer;

    return engine.run( &layer, 1 );
}
