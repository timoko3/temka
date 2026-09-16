#ifndef ENGINE_MATH_VEC2_HH
#define ENGINE_MATH_VEC2_HH

// 2D-вектор (architecture.md §5.2). constexpr — где не требуется sqrt
// (length/normalize — обычные функции, sqrt в C++17 не constexpr).

namespace tml::math
{

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2()                                = default;
    constexpr Vec2( float px, float py )            : x( px ), y( py ) {}

    constexpr Vec2 operator+( const Vec2 & o ) const { return Vec2{ x + o.x, y + o.y }; }
    constexpr Vec2 operator-( const Vec2 & o ) const { return Vec2{ x - o.x, y - o.y }; }
    constexpr Vec2 operator-()                 const { return Vec2{ -x, -y }; }
    constexpr Vec2 operator*( float s )        const { return Vec2{ x * s, y * s }; }
    constexpr Vec2 operator/( float s )        const { return Vec2{ x / s, y / s }; }

    Vec2 & operator+=( const Vec2 & o )
    {
        x += o.x;
        y += o.y;
        return *this;
    }

    Vec2 & operator-=( const Vec2 & o )
    {
        x -= o.x;
        y -= o.y;
        return *this;
    }

    Vec2 & operator*=( float s )
    {
        x *= s;
        y *= s;
        return *this;
    }

    constexpr float dot( const Vec2 & o ) const
    {
        return x * o.x + y * o.y;
    }

    float length() const;

    // Нулевой вектор нормализуется в нулевой — без UB.
    Vec2 normalize() const;

    constexpr bool operator==( const Vec2 & o ) const { return x == o.x && y == o.y; }
    constexpr bool operator!=( const Vec2 & o ) const { return !( *this == o ); }
};

inline float Vec2::length() const
{
    return __builtin_sqrt( x * x + y * y );
}

inline Vec2 Vec2::normalize() const
{
    const float len = length();
    if ( len == 0.0f )
    {
        return Vec2{ 0.0f, 0.0f };
    }
    return Vec2{ x / len, y / len };
}

inline constexpr Vec2 lerp( const Vec2 & a, const Vec2 & b, float t )
{
    return Vec2{ a.x + ( b.x - a.x ) * t, a.y + ( b.y - a.y ) * t };
}

} // namespace tml::math

#endif // ENGINE_MATH_VEC2_HH
