#ifndef ENGINE_RENDER_IRENDERER_HH
#define ENGINE_RENDER_IRENDERER_HH

#include "tml/core/uniquePtr.hh"
#include "tml/platform/IPlatform.hh"
#include "tml/platform/windowDesc.hh"
#include "tml/render/IDebugDraw.hh"

// Общий интерфейс рендера (architecture.md §5.6).
//
// Бэкенд предоставляет фабрику createRenderer(), чтобы Engine не зависел
// от конкретной реализации (sfml/gl/vk).

namespace tml::rnd
{

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual bool init( IPlatform & platform, const WindowDesc & desc ) = 0;
    virtual void shutdown()                                            = 0;

    virtual void beginFrame()                                          = 0;
    virtual void endFrame()                                            = 0;   // present

    virtual IDebugDraw & debug()                                       = 0;

    virtual const char * backendName() const                           = 0;
};

UniquePtr< IRenderer > createRenderer();

} // namespace tml::rnd

#endif // ENGINE_RENDER_IRENDERER_HH
