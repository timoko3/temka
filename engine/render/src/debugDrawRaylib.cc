#include "debugDrawRaylib.hh"

#include <algorithm>
#include <cmath>
#include <raylib.h>

namespace tml::rnd
{

namespace
{

::Color toRaylib( Color c )
{
    return {
        static_cast< unsigned char >( std::min( 255.0f, c.r * 255.0f ) ),
        static_cast< unsigned char >( std::min( 255.0f, c.g * 255.0f ) ),
        static_cast< unsigned char >( std::min( 255.0f, c.b * 255.0f ) ),
        static_cast< unsigned char >( std::min( 255.0f, c.a * 255.0f ) )
    };
}

} // anonymous namespace

void DebugDrawRaylib::begin( const Camera & camera )
{
    camera_        = camera;
    pixelsPerUnit_ = static_cast< float >( GetScreenHeight() ) * camera_.viewProj.at( 1, 1 ) * 0.5f;
    if ( pixelsPerUnit_ < 1.0e-6f )
    {
        pixelsPerUnit_ = 1.0f;
    }
}

void DebugDrawRaylib::end()
{
    // Для immediate-mode raylib ничего сбрасывать не нужно.
}

math::Vec2 DebugDrawRaylib::worldToScreen( math::Vec3 p ) const
{
    const math::Vec4 clip = camera_.viewProj * math::Vec4{ p.x, p.y, p.z, 1.0f };
    const float    invW   = 1.0f / clip.w;
    const float    ndcX   = clip.x * invW;
    const float    ndcY   = clip.y * invW;

    const float screenW = static_cast< float >( GetScreenWidth() );
    const float screenH = static_cast< float >( GetScreenHeight() );

    return math::Vec2{ ( ndcX + 1.0f ) * 0.5f * screenW,
                       ( 1.0f - ndcY ) * 0.5f * screenH };
}

void DebugDrawRaylib::point( math::Vec3 p, Color c )
{
    circle( p, 0.05f, c );
}

void DebugDrawRaylib::line( math::Vec3 a, math::Vec3 b, Color c )
{
    const math::Vec2 sa = worldToScreen( a );
    const math::Vec2 sb = worldToScreen( b );
    DrawLineV( Vector2{ sa.x, sa.y }, Vector2{ sb.x, sb.y }, toRaylib( c ) );
}

void DebugDrawRaylib::polyline( const math::Vec3 * pts, std::uint32_t n,
                                Color c, bool closed )
{
    if ( n < 2 )
    {
        return;
    }
    for ( std::uint32_t i = 0; i + 1 < n; ++i )
    {
        line( pts[ i ], pts[ i + 1 ], c );
    }
    if ( closed )
    {
        line( pts[ n - 1 ], pts[ 0 ], c );
    }
}

void DebugDrawRaylib::circle( math::Vec3 center, float radius, Color c )
{
    const math::Vec2 s = worldToScreen( center );
    const float      r = std::max( 1.0f, radius * pixelsPerUnit_ );
    DrawCircleV( Vector2{ s.x, s.y }, r, toRaylib( c ) );
}

void DebugDrawRaylib::aabb( const math::AABB & box, Color c )
{
    const math::Vec3 corners[ 5 ] = {
        math::Vec3{ box.min.x, box.min.y, 0.0f },
        math::Vec3{ box.max.x, box.min.y, 0.0f },
        math::Vec3{ box.max.x, box.max.y, 0.0f },
        math::Vec3{ box.min.x, box.max.y, 0.0f },
        math::Vec3{ box.min.x, box.min.y, 0.0f }
    };
    polyline( corners, 5, c, false );
}

void DebugDrawRaylib::sphere( math::Vec3 center, float radius, Color c )
{
    // В 2D-заглушке сфера рисуется как круг в плоскости XY.
    circle( center, radius, c );
}

void DebugDrawRaylib::triad( math::Vec3 origin, float size )
{
    arrow( origin, math::Vec3{ origin.x + size, origin.y, origin.z }, Red() );
    arrow( origin, math::Vec3{ origin.x, origin.y + size, origin.z }, Green() );
    arrow( origin, math::Vec3{ origin.x, origin.y, origin.z + size }, Blue() );
}

void DebugDrawRaylib::arrow( math::Vec3 from, math::Vec3 to, Color c )
{
    line( from, to, c );

    // Упрощённая стрелка: короткая перпендикулярная линия у конца.
    const math::Vec2 sf = worldToScreen( from );
    const math::Vec2 st = worldToScreen( to );
    const math::Vec2 dir{ st.x - sf.x, st.y - sf.y };
    const float      len = std::sqrt( dir.x * dir.x + dir.y * dir.y );
    if ( len < 1.0e-6f )
    {
        return;
    }

    const math::Vec2 n{ -dir.y / len, dir.x / len };
    const float      head = 6.0f;   // пиксели
    const math::Vec2 tail{ st.x - dir.x * ( head / len ),
                            st.y - dir.y * ( head / len ) };

    DrawLineV( Vector2{ st.x, st.y },
               Vector2{ tail.x + n.x * head * 0.5f, tail.y + n.y * head * 0.5f },
               toRaylib( c ) );
    DrawLineV( Vector2{ st.x, st.y },
               Vector2{ tail.x - n.x * head * 0.5f, tail.y - n.y * head * 0.5f },
               toRaylib( c ) );
}

void DebugDrawRaylib::text( math::Vec3 pos, const char * utf8, Color c )
{
    const math::Vec2 s = worldToScreen( pos );
    DrawText( utf8, static_cast< int >( s.x ), static_cast< int >( s.y ), 20, toRaylib( c ) );
}

} // namespace tml::rnd
