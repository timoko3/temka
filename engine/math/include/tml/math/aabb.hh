#ifndef ENGINE_MATH_AABB_HH
#define ENGINE_MATH_AABB_HH

#include "tml/math/vec3.hh"

#include <cfloat>

// Ось-выровненный параллелепипед (architecture.md §5.2).
//
// По умолчанию — «пустой» бокс (инвертированный): min = +MAX, max = -MAX.
// После expand() хотя бы одной точки становится обычным. Такой выбор делает
// типичный цикл построения бокса по облаку точек тривиальным:
//     AABB box;
//     for ( p : points ) box.expand( p );

// Это квадрат, всегда выровненный по осям и обхватывающий весь объект

namespace tml::math
{

struct AABB
{
    Vec3 min{  FLT_MAX,  FLT_MAX,  FLT_MAX };
    Vec3 max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

    // Включение точки в паралелипипед
    void expand( const Vec3 & p )
    {
        if ( p.x < min.x ) { min.x = p.x; }
        if ( p.y < min.y ) { min.y = p.y; }
        if ( p.z < min.z ) { min.z = p.z; }
        if ( p.x > max.x ) { max.x = p.x; }
        if ( p.y > max.y ) { max.y = p.y; }
        if ( p.z > max.z ) { max.z = p.z; }

        if ( max.x < min.x ) { max.x = min.x; }   // первая точка: min == max
        if ( max.y < min.y ) { max.y = min.y; }
        if ( max.z < min.z ) { max.z = min.z; }
    }

    // Объедиение двух паралелипипедов
    void expand( const AABB & o )
    {
        expand( o.min );
        expand( o.max );
    }

    // Пересечение по правилу «проекции на все оси пересекаются».
    bool intersects( const AABB & o ) const
    {
        return min.x <= o.max.x && max.x >= o.min.x
            && min.y <= o.max.y && max.y >= o.min.y
            && min.z <= o.max.z && max.z >= o.min.z;
    }

    // есть ли точка внутри
    bool contains( const Vec3 & p ) const
    {
        return p.x >= min.x && p.x <= max.x
            && p.y >= min.y && p.y <= max.y
            && p.z >= min.z && p.z <= max.z;
    }

    Vec3 center() const
    {
        return Vec3{ ( min.x + max.x ) * 0.5f,
                     ( min.y + max.y ) * 0.5f,
                     ( min.z + max.z ) * 0.5f };
    }

    Vec3 extent() const
    {
        return Vec3{ ( max.x - min.x ) * 0.5f,
                     ( max.y - min.y ) * 0.5f,
                     ( max.z - min.z ) * 0.5f };
    }
};

} // namespace tml::math

#endif // ENGINE_MATH_AABB_HH
