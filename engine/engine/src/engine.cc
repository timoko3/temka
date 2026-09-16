#include "tml/engine/engine.hh"

#include "tml/core/log.hh"
#include "tml/engine/layer.hh"
#include "tml/platform/IPlatform.hh"
#include "tml/render/IRenderer.hh"

#include <algorithm>
#include <cmath>

namespace tml
{

int Engine::run( Layer * layers, std::size_t layerCount )
{
    platform_ = createPlatform();
    if ( !platform_->init( config_.window ) )
    {
        TM_ERROR( "engine", "platform init failed" );
        return -1;
    }

    renderer_ = rnd::createRenderer();
    if ( !renderer_->init( *platform_, config_.window ) )
    {
        TM_ERROR( "engine", "renderer init failed" );
        return -2;
    }

    log::setLevel( config_.logLevel );
    TM_INFO( "engine", "running with fixedDt=%.4f, backend=%s",
             config_.fixedDt, renderer_->backendName() );

    for ( std::size_t i = 0; i < layerCount; ++i )
    {
        layers[ i ].onAttach( *this );
    }

    double prevTime     = platform_->timeSeconds();
    float  accumulator  = 0.0f;

    while ( platform_->pollEvents() )
    {
        const double currTime = platform_->timeSeconds();
        float frameDelta      = static_cast< float >( currTime - prevTime );
        prevTime              = currTime;

        // Защита от огромного dt при паузе/отладке.
        frameDelta = std::min( frameDelta, 0.25f );

        accumulator += frameDelta;

        unsigned int steps = 0;
        while ( accumulator >= config_.fixedDt && steps < config_.maxStepsPerFrame )
        {
            for ( std::size_t i = 0; i < layerCount; ++i )
            {
                layers[ i ].onFixedUpdate( config_.fixedDt );
            }
            accumulator -= config_.fixedDt;
            ++steps;
        }

        const float alpha = accumulator / config_.fixedDt;

        renderer_->beginFrame();
        for ( std::size_t i = 0; i < layerCount; ++i )
        {
            layers[ i ].onUpdate( frameDelta, alpha );
            layers[ i ].onRender( *renderer_ );
        }
        renderer_->endFrame();
    }

    for ( std::size_t i = 0; i < layerCount; ++i )
    {
        layers[ i ].onDetach();
    }

    renderer_->shutdown();
    return 0;
}

} // namespace tml
