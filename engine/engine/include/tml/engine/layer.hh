#ifndef ENGINE_ENGINE_LAYER_HH
#define ENGINE_ENGINE_LAYER_HH

#include "tml/render/IRenderer.hh"

// Слой прикладной логики (architecture.md §5.10): playground и редактор —
// это наборы слоёв поверх Engine.
//
// onEvent отсутствует на этом этапе, потому что EventBus реализуется позже
// (аддитивно, не ломает архитектуру).

namespace tml
{

class Engine;

class Layer
{
public:
    virtual ~Layer() = default;

    virtual void onAttach( Engine & /*engine*/ ) {}
    virtual void onDetach() {}

    virtual void onFixedUpdate( float /*fixedDt*/ ) {}
    virtual void onUpdate( float /*dt*/, float /*alpha*/ ) {}
    virtual void onRender( rnd::IRenderer & /*renderer*/ ) {}
};

} // namespace tml

#endif // ENGINE_ENGINE_LAYER_HH
