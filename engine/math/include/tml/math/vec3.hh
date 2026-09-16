#ifndef ENGINE_MATH_VEC3_HH
#define ENGINE_MATH_VEC3_HH

// 3D-вектор (architecture.md §5.2): кроме Vec2-операций — cross.

namespace tml::math
{

struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3()                                          = default;
    constexpr Vec3( float px, float py, float pz )            : x( px ), y( py ), z( pz ) {}

    constexpr Vec3 operator+( const Vec3 & o ) const { return Vec3{ x + o.x, y + o.y, z + o.z }; }
    constexpr Vec3 operator-( const Vec3 & o ) const { return Vec3{ x - o.x, y - o.y, z - o.z }; }
    constexpr Vec3 operator-()                 const { return Vec3{ -x, -y, -z }; }
    constexpr Vec3 operator*( float s )        const { return Vec3{ x * s, y * s, z * s }; }
    constexpr Vec3 operator/( float s )        const { return Vec3{ x / s, y / s, z / s }; }

    Vec3 & operator+=( const Vec3 & o )
    {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }

    Vec3 & operator-=( const Vec3 & o )
    {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        return *this;
    }

    Vec3 & operator*=( float s )
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    constexpr float dot( const Vec3 & o ) const
    {
        return x * o.x + y * o.y + z * o.z;
    }

    constexpr Vec3 cross( const Vec3 & o ) const
    {
        return Vec3{ y * o.z - z * o.y,
                     z * o.x - x * o.z,
                     x * o.y - y * o.x };
    }

    float length() const;

    // Нулевой вектор нормализуется в нулевой — без UB.
    Vec3 normalize() const;

    constexpr bool operator==( const Vec3 & o ) const { return x == o.x && y == o.y && z == o.z; }
    constexpr bool operator!=( const Vec3 & o ) const { return !( *this == o ); }
};

inline float Vec3::length() const
{
    return __builtin_sqrt( x * x + y * y + z * z );
}

inline Vec3 Vec3::normalize() const
{
    const float len = length();
    if ( len == 0.0f )
    {
        return Vec3{ 0.0f, 0.0f, 0.0f };
    }
    return Vec3{ x / len, y / len, z / len };
}

inline constexpr Vec3 lerp( const Vec3 & a, const Vec3 & b, float t )
{
    return Vec3{ a.x + ( b.x - a.x ) * t,
                 a.y + ( b.y - a.y ) * t,
                 a.z + ( b.z - a.z ) * t };
}

} // namespace tml::math

#endif // ENGINE_MATH_VEC3_HH
