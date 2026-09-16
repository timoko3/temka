#include "tml/platform/IPlatform.hh"

#include "tml/core/log.hh"

#include <raylib.h>

namespace tml
{

class PlatformRaylib : public IPlatform
{
public:
    PlatformRaylib() = default;

    ~PlatformRaylib() override
    {
        if ( windowOpened_ )
        {
            CloseWindow();
            windowOpened_ = false;
        }
    }

    bool init( const WindowDesc & desc ) override
    {
        SetConfigFlags( FLAG_WINDOW_RESIZABLE );
        InitWindow( static_cast< int >( desc.width ),
                    static_cast< int >( desc.height ),
                    desc.title );
        SetTargetFPS( 60 );
        windowOpened_ = true;

        TM_INFO( "platform", "raylib window opened: %ux%u", desc.width, desc.height );
        return true;
    }

    bool pollEvents() override
    {
        // raylib обновляет внутреннее состояние окна при вызове EndDrawing(),
        // но PollInputEvents() гарантирует актуальность событий ввода.
        PollInputEvents();

        // Поддержка Alt+F4, если оконный менеджер не выставил WindowShouldClose().
        if ( IsKeyDown( KEY_LEFT_ALT ) && IsKeyPressed( KEY_F4 ) )
        {
            closeRequested_ = true;
        }

        return windowOpened_ && !closeRequested_ && !WindowShouldClose();
    }

    const InputState & input() const override
    {
        // ЗАГЛУШКА этапа: снимок ввода возвращается нулевым.
        // Заполнение из raylib добавится позже.
        static InputState stub;
        return stub;
    }

    void * nativeWindowHandle() override
    {
        return GetWindowHandle();
    }

    double timeSeconds() const override
    {
        return GetTime();
    }

    void requestClose() override
    {
        closeRequested_ = true;
    }

private:
    bool windowOpened_    = false;
    bool closeRequested_  = false;
};

UniquePtr< IPlatform > createPlatform()
{
    return UniquePtr< IPlatform >( new PlatformRaylib() );
}

} // namespace tml
