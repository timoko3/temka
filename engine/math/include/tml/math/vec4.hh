#ifndef ENGINE_MATH_VEC4_HH
#define ENGINE_MATH_VEC4_HH

// 4D-вектор (architecture.md §5.2): в основном для однородных координат
// с Mat4 (w-компонента).

namespace tml::math
{

struct Vec4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    constexpr Vec4()                                                     = default;
    constexpr Vec4( float px, float py, float pz, float pw )             : x( px ), y( py ), z( pz ), w( pw ) {}

    constexpr Vec4 operator+( const Vec4 & o ) const { return Vec4{ x + o.x, y + o.y, z + o.z, w + o.w }; }
    constexpr Vec4 operator-( const Vec4 & o ) const { return Vec4{ x - o.x, y - o.y, z - o.z, w - o.w }; }
    constexpr Vec4 operator-()                 const { return Vec4{ -x, -y, -z, -w }; }
    constexpr Vec4 operator*( float s )        const { return Vec4{ x * s, y * s, z * s, w * s }; }

    constexpr float dot( const Vec4 & o ) const
    {
        return x * o.x + y * o.y + z * o.z + w * o.w;
    }

    float length() const;

    constexpr bool operator==( const Vec4 & o ) const
    {
        return x == o.x && y == o.y && z == o.z && w == o.w;
    }
    constexpr bool operator!=( const Vec4 & o ) const { return !( *this == o ); }
};

inline float Vec4::length() const
{
    return __builtin_sqrt( x * x + y * y + z * z + w * w );
}

} // namespace tml::math

#endif // ENGINE_MATH_VEC4_HH
