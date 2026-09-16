#ifndef ENGINE_RENDER_COLOR_HH
#define ENGINE_RENDER_COLOR_HH

// Цвет в линейном float (architecture.md §5.6).

namespace tml::rnd
{

struct Color
{
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

inline constexpr Color makeColor( float r, float g, float b, float a = 1.0f )
{
    return Color{ r, g, b, a };
}

inline constexpr Color Red()    { return makeColor( 1.0f, 0.0f, 0.0f ); }
inline constexpr Color Green()  { return makeColor( 0.0f, 1.0f, 0.0f ); }
inline constexpr Color Blue()   { return makeColor( 0.0f, 0.0f, 1.0f ); }
inline constexpr Color White()  { return makeColor( 1.0f, 1.0f, 1.0f ); }
inline constexpr Color Black()  { return makeColor( 0.0f, 0.0f, 0.0f ); }
inline constexpr Color Yellow() { return makeColor( 1.0f, 1.0f, 0.0f ); }

} // namespace tml::rnd

#endif // ENGINE_RENDER_COLOR_HH
