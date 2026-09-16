#ifndef ENGINE_PLATFORM_INPUT_HH
#define ENGINE_PLATFORM_INPUT_HH

#include "tml/math/vec2.hh"

#include <cstdint>

// Снимок ввода на один кадр (architecture.md §5.3).
//
// ЗАГЛУШКА этапа: структура определена, но на этом этапе реализация
// platform_raylib возвращает нулевой снимок. Реальное опросное заполнение
// добавится позже без изменения API.

namespace tml
{

enum class Key : std::uint32_t
{
    Unknown = 0,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Space, Escape, Enter, Tab, Backspace,
    Left, Right, Up, Down,
    LShift, LCtrl, LAlt,
    Count
};

enum class MouseButton : std::uint8_t
{
    Left = 0,
    Right,
    Middle,
    Count
};

struct InputState
{
    bool keyDown[ static_cast< std::size_t >( Key::Count ) ]      = {};
    bool keyPressed[ static_cast< std::size_t >( Key::Count ) ]   = {};
    bool mouseDown[ static_cast< std::size_t >( MouseButton::Count ) ] = {};

    math::Vec2 mousePos;
    math::Vec2 mouseDelta;
    math::Vec2 wheelDelta;
};

} // namespace tml

#endif // ENGINE_PLATFORM_INPUT_HH
