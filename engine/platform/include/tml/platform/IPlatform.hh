#ifndef ENGINE_PLATFORM_IPLATFORM_HH
#define ENGINE_PLATFORM_IPLATFORM_HH

#include "tml/core/uniquePtr.hh"
#include "tml/platform/input.hh"
#include "tml/platform/windowDesc.hh"

// Единственный шов к ОС/библиотеке окна (architecture.md §5.3).
//
// Реализации (platform_raylib, в будущем platform_sfml/glfw) предоставляют
// фабрику createPlatform(), чтобы Engine не зависел от конкретного бэкенда.

namespace tml
{

class IPlatform
{
public:
    virtual ~IPlatform() = default;

    virtual bool init( const WindowDesc & desc )           = 0;
    virtual bool pollEvents()                              = 0;   // false → окно закрыто
    virtual const InputState & input() const               = 0;
    virtual void * nativeWindowHandle()                    = 0;   // для GL/Vulkan-контекста
    virtual double timeSeconds() const                     = 0;
    virtual void requestClose()                            = 0;
};

UniquePtr< IPlatform > createPlatform();

} // namespace tml

#endif // ENGINE_PLATFORM_IPLATFORM_HH
