#include "testFramework.h"

#include "tml/math/aabb.hh"
#include "tml/math/mat4.hh"
#include "tml/math/vec2.hh"
#include "tml/math/vec3.hh"
#include "tml/math/vec4.hh"

namespace
{

// Сравнение float с эпсилоном (modulePlan §2: свой хелпер для тестов).
bool approx( float a, float b, float eps = 1.0e-5f )
{
    return a - b < eps && b - a < eps;
}

bool approxVec( tml::math::Vec2 v, float x, float y )
{
    return approx( v.x, x ) && approx( v.y, y );
}

bool approxVec( tml::math::Vec3 v, float x, float y, float z )
{
    return approx( v.x, x ) && approx( v.y, y ) && approx( v.z, z );
}

bool approxVec( tml::math::Vec4 v, float x, float y, float z, float w )
{
    return approx( v.x, x ) && approx( v.y, y ) && approx( v.z, z ) && approx( v.w, w );
}

bool approxMatIdentity( const tml::math::Mat4 & m )
{
    for ( unsigned row = 0; row < 4; ++row )
    {
        for ( unsigned col = 0; col < 4; ++col )
        {
            const float expected = ( row == col ) ? 1.0f : 0.0f;
            if ( !approx( m.at( row, col ), expected ) )
            {
                return false;
            }
        }
    }
    return true;
}

} // anonymous namespace

using namespace tml::math;

TM_TEST( vec2Arithmetic )
{
    const Vec2 a{ 1.0f, 2.0f };
    const Vec2 b{ 3.0f, 4.0f };

    TM_CHECK( approxVec( a + b, 4.0f, 6.0f ) );
    TM_CHECK( approxVec( b - a, 2.0f, 2.0f ) );
    TM_CHECK( approxVec( -a, -1.0f, -2.0f ) );
    TM_CHECK( approxVec( a * 3.0f, 3.0f, 6.0f ) );
    TM_CHECK( approxVec( b / 2.0f, 1.5f, 2.0f ) );

    Vec2 c{ 1.0f, 1.0f };
    c += Vec2{ 2.0f, 3.0f };
    TM_CHECK( approxVec( c, 3.0f, 4.0f ) );
    c -= Vec2{ 1.0f, 1.0f };
    TM_CHECK( approxVec( c, 2.0f, 3.0f ) );
    c *= 2.0f;
    TM_CHECK( approxVec( c, 4.0f, 6.0f ) );
}

TM_TEST( vec2Geometry )
{
    // dot: {1,2}·{3,4} = 3 + 8 = 11
    TM_CHECK( approx( Vec2{ 1.0f, 2.0f }.dot( Vec2{ 3.0f, 4.0f } ), 11.0f ) );

    // length: {3,4} = 5
    TM_CHECK( approx( Vec2{ 3.0f, 4.0f }.length(), 5.0f ) );

    // normalize: {3,4} → {0.6, 0.8}, длина 1
    const Vec2 n = Vec2{ 3.0f, 4.0f }.normalize();
    TM_CHECK( approxVec( n, 0.6f, 0.8f ) );
    TM_CHECK( approx( n.length(), 1.0f ) );

    // нулевой вектор нормализуется в нулевой
    TM_CHECK( approxVec( Vec2{ 0.0f, 0.0f }.normalize(), 0.0f, 0.0f ) );

    // lerp( a, b, 0.5 ) — середина
    TM_CHECK( approxVec( lerp( Vec2{ 0.0f, 0.0f }, Vec2{ 10.0f, 10.0f }, 0.5f ), 5.0f, 5.0f ) );
    TM_CHECK( approxVec( lerp( Vec2{ 2.0f, 0.0f }, Vec2{ 4.0f, 8.0f }, 0.25f ), 2.5f, 2.0f ) );
}

TM_TEST( vec3Cross )
{
    // Базисные векторы: x × y = z, y × z = x, z × x = y; обратный порядок — минус.
    const Vec3 x{ 1.0f, 0.0f, 0.0f };
    const Vec3 y{ 0.0f, 1.0f, 0.0f };
    const Vec3 z{ 0.0f, 0.0f, 1.0f };

    TM_CHECK( approxVec( x.cross( y ), 0.0f, 0.0f, 1.0f ) );
    TM_CHECK( approxVec( y.cross( x ), 0.0f, 0.0f, -1.0f ) );
    TM_CHECK( approxVec( y.cross( z ), 1.0f, 0.0f, 0.0f ) );
    TM_CHECK( approxVec( z.cross( x ), 0.0f, 1.0f, 0.0f ) );

    // {1,2,3} × {4,5,6} = {2*6-3*5, 3*4-1*6, 1*5-2*4} = {-3, 6, -3}
    TM_CHECK( approxVec( Vec3{ 1.0f, 2.0f, 3.0f }.cross( Vec3{ 4.0f, 5.0f, 6.0f } ),
                         -3.0f, 6.0f, -3.0f ) );

    // Параметрическое property: perpendicular к обоим — dot == 0
    const Vec3 a{ 1.0f, -2.0f, 3.0f };
    const Vec3 b{ 4.0f, 1.0f, -2.0f };
    const Vec3 c = a.cross( b );
    TM_CHECK( approx( c.dot( a ), 0.0f ) );
    TM_CHECK( approx( c.dot( b ), 0.0f ) );
}

TM_TEST( vec4Basics )
{
    const Vec4 a{ 1.0f, 2.0f, 3.0f, 4.0f };
    const Vec4 b{ 5.0f, 6.0f, 7.0f, 8.0f };

    TM_CHECK( approxVec( a + b, 6.0f, 8.0f, 10.0f, 12.0f ) );
    // dot: 5 + 12 + 21 + 32 = 70
    TM_CHECK( approx( a.dot( b ), 70.0f ) );
    // length {1,2,3,4} = sqrt(30)
    TM_CHECK( approx( a.length(), __builtin_sqrt( 30.0f ) ) );
}

TM_TEST( mat4Identity )
{
    const Mat4 id = Mat4::identity();
    TM_CHECK( approxMatIdentity( id ) );

    // I * M == M, I * v == v
    const Mat4 p = Mat4::perspective( 1.0f, 1.5f, 0.1f, 100.0f );
    const Mat4 r = id * p;
    for ( unsigned row = 0; row < 4; ++row )
    {
        for ( unsigned col = 0; col < 4; ++col )
        {
            TM_CHECK( approx( r.at( row, col ), p.at( row, col ) ) );
        }
    }

    const Vec4 v{ 1.0f, 2.0f, 3.0f, 1.0f };
    TM_CHECK( approxVec( id * v, 1.0f, 2.0f, 3.0f, 1.0f ) );
}

TM_TEST( mat4Transpose )
{
    Mat4 m;
    float value = 0.0f;
    for ( unsigned col = 0; col < 4; ++col )
    {
        for ( unsigned row = 0; row < 4; ++row )
        {
            m.at( row, col ) = value;
            value += 1.0f;
        }
    }

    const Mat4 t = m.transpose();
    for ( unsigned row = 0; row < 4; ++row )
    {
        for ( unsigned col = 0; col < 4; ++col )
        {
            TM_CHECK( approx( t.at( row, col ), m.at( col, row ) ) );
        }
    }

    // Двойная транспозиция — исходная.
    const Mat4 tt = t.transpose();
    for ( unsigned row = 0; row < 4; ++row )
    {
        for ( unsigned col = 0; col < 4; ++col )
        {
            TM_CHECK( approx( tt.at( row, col ), m.at( row, col ) ) );
        }
    }
}

TM_TEST( mat4InverseProperty )
{
    // Ключевое свойство (modulePlan §2): inverse( M ) * M ≈ I.
    const Mat4 matrices[] = {
        Mat4::perspective( 1.2f, 1.7777f, 0.1f, 100.0f ),
        Mat4::ortho( -10.0f, 10.0f, -5.0f, 5.0f, 0.1f, 50.0f ),
        Mat4::lookAt( Vec3{ 3.0f, 2.0f, 5.0f }, Vec3{ 0.0f, 0.5f, 0.0f }, Vec3{ 0.0f, 1.0f, 0.0f } ),
    };

    for ( const Mat4 & m : matrices )
    {
        const Mat4 inv    = m.inverse();
        const Mat4 check1 = inv * m;
        const Mat4 check2 = m * inv;
        TM_CHECK( approxMatIdentity( check1 ) );
        TM_CHECK( approxMatIdentity( check2 ) );
    }
}

TM_TEST( mat4MulAssociativity )
{
    // ( AB )C == A( BC ) — структурное свойство, никаких эталонов руками.
    const Mat4 a = Mat4::ortho( -2.0f, 6.0f, -1.0f, 4.0f, 1.0f, 20.0f );
    const Mat4 b = Mat4::lookAt( Vec3{ 1.0f, 1.0f, 3.0f }, Vec3{ 0.0f, 0.0f, 0.0f }, Vec3{ 0.0f, 1.0f, 0.0f } );
    const Mat4 c = Mat4::perspective( 0.9f, 1.0f, 0.5f, 30.0f );

    const Mat4 ab_c = ( a * b ) * c;
    const Mat4 a_bc = a * ( b * c );

    for ( unsigned row = 0; row < 4; ++row )
    {
        for ( unsigned col = 0; col < 4; ++col )
        {
            TM_CHECK( approx( ab_c.at( row, col ), a_bc.at( row, col ), 1.0e-4f ) );
        }
    }
}

TM_TEST( mat4OrthoReference )
{
    // ortho( -10, 10, -5, 5, 0.1, 50 ): ручные эталонные значения.
    const Mat4 m = Mat4::ortho( -10.0f, 10.0f, -5.0f, 5.0f, 0.1f, 50.0f );

    TM_CHECK( approx( m.at( 0, 0 ), 2.0f / 20.0f ) );      // 0.1
    TM_CHECK( approx( m.at( 1, 1 ), 2.0f / 10.0f ) );      // 0.2
    TM_CHECK( approx( m.at( 2, 2 ), -2.0f / 49.9f ) );
    TM_CHECK( approx( m.at( 0, 3 ), 0.0f ) );              // симметрично по x
    TM_CHECK( approx( m.at( 1, 3 ), 0.0f ) );              // симметрично по y
    TM_CHECK( approx( m.at( 2, 3 ), -50.1f / 49.9f ) );
    TM_CHECK( approx( m.at( 3, 3 ), 1.0f ) );

    // Углы бокса маппятся в углы NDC: ( left, bottom ) -> ( -1, -1 ).
    const Vec4 p{ -10.0f, -5.0f, 0.0f, 1.0f };
    const Vec4 r = m * p;
    TM_CHECK( approxVec( r, -1.0f, -1.0f, r.z, 1.0f ) );
}

TM_TEST( mat4LookAtReference )
{
    // Камера на (0,0,10), смотрит в начало координат, up = +Y.
    const Mat4 v = Mat4::lookAt( Vec3{ 0.0f, 0.0f, 10.0f },
                                 Vec3{ 0.0f, 0.0f, 0.0f },
                                 Vec3{ 0.0f, 1.0f, 0.0f } );

    // Ожидаемая view-матрица: x → x, y → y, z → z - 10.
    TM_CHECK( approx( v.at( 0, 0 ), 1.0f ) );
    TM_CHECK( approx( v.at( 1, 1 ), 1.0f ) );
    TM_CHECK( approx( v.at( 2, 2 ), 1.0f ) );
    TM_CHECK( approx( v.at( 2, 3 ), -10.0f ) );

    // Позиция камеры должна маппиться в начало координат.
    const Vec4 eye{ 0.0f, 0.0f, 10.0f, 1.0f };
    TM_CHECK( approxVec( v * eye, 0.0f, 0.0f, 0.0f, 1.0f ) );
}

TM_TEST( aabbExpand )
{
    AABB box;
    box.expand( Vec3{ 1.0f, 2.0f, 3.0f } );
    TM_CHECK( approxVec( box.min, 1.0f, 2.0f, 3.0f ) );
    TM_CHECK( approxVec( box.max, 1.0f, 2.0f, 3.0f ) );

    box.expand( Vec3{ -1.0f, 5.0f, 0.5f } );
    TM_CHECK( approxVec( box.min, -1.0f, 2.0f, 0.5f ) );
    TM_CHECK( approxVec( box.max, 1.0f, 5.0f, 3.0f ) );

    // Внутренняя точка ничего не меняет.
    box.expand( Vec3{ 0.0f, 3.0f, 1.0f } );
    TM_CHECK( approxVec( box.min, -1.0f, 2.0f, 0.5f ) );
    TM_CHECK( approxVec( box.max, 1.0f, 5.0f, 3.0f ) );
}

TM_TEST( aabbIntersects )
{
    AABB a;
    a.expand( Vec3{ 0.0f, 0.0f, 0.0f } );
    a.expand( Vec3{ 2.0f, 2.0f, 2.0f } );

    AABB b;
    b.expand( Vec3{ 1.0f, 1.0f, 1.0f } );
    b.expand( Vec3{ 3.0f, 3.0f, 3.0f } );

    AABB far;
    far.expand( Vec3{ 2.0f, 2.0f, 5.0f } );   // отделён по z
    far.expand( Vec3{ 3.0f, 3.0f, 6.0f } );

    TM_CHECK( a.intersects( b ) );
    TM_CHECK( b.intersects( a ) );
    TM_CHECK( !a.intersects( far ) );

    // Касание гранями считается пересечением ( <= / >= ).
    AABB touch;
    touch.expand( Vec3{ 2.0f, 0.0f, 0.0f } );
    touch.expand( Vec3{ 4.0f, 2.0f, 2.0f } );
    TM_CHECK( a.intersects( touch ) );

    // contains / center / extent.
    TM_CHECK( a.contains( Vec3{ 1.0f, 1.0f, 1.0f } ) );
    TM_CHECK( !a.contains( Vec3{ 3.0f, 1.0f, 1.0f } ) );
    TM_CHECK( approxVec( a.center(), 1.0f, 1.0f, 1.0f ) );
    TM_CHECK( approxVec( a.extent(), 1.0f, 1.0f, 1.0f ) );
}
