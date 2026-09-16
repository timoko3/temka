#ifndef ENGINE_PLATFORM_WINDOW_DESC_HH
#define ENGINE_PLATFORM_WINDOW_DESC_HH

// Описание окна (architecture.md §5.3).

namespace tml
{

struct WindowDesc
{
    const char * title      = "temka";
    unsigned int width      = 1280;
    unsigned int height     = 720;
    bool         resizable  = true;
    bool         vsync      = true;
};

} // namespace tml

#endif // ENGINE_PLATFORM_WINDOW_DESC_HH
