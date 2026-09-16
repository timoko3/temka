#ifndef ENGINE_MATH_MAT4_HH
#define ENGINE_MATH_MAT4_HH

#include "tml/core/assert.hh"
#include "tml/math/vec3.hh"
#include "tml/math/vec4.hh"

#include <cstdint>

// Матрица 4×4, column-major (как в OpenGL, architecture.md §5.2):
// элемент (row, col) хранится в m[col * 4 + row].
//
// Умножение матрицы на вектор — в однородных координатах:
// результат mat * Vec4{ v, 1 } учитывает и поворот, и перенос.
//
// lookAt/perspective — правая система координат, камера смотрит вдоль -Z.

namespace tml::math
{

struct Mat4
{
    float m[16] = {};   // все нули по умолчанию

    // Доступ к элементу: at( строка, столбец ).
    float & at( uint32_t row, uint32_t col )
    {
        return m[ col * 4 + row ];
    }

    const float & at( uint32_t row, uint32_t col ) const
    {
        return m[ col * 4 + row ];
    }

    static Mat4 identity()
    {
        Mat4 r;
        r.at( 0, 0 ) = 1.0f;
        r.at( 1, 1 ) = 1.0f;
        r.at( 2, 2 ) = 1.0f;
        r.at( 3, 3 ) = 1.0f;
        return r;
    }

    Mat4 operator*( const Mat4 & o ) const
    {
        Mat4 r;
        for ( uint32_t col = 0; col < 4; ++col )
        {
            for ( uint32_t row = 0; row < 4; ++row )
            {
                float sum = 0.0f;
                for ( uint32_t k = 0; k < 4; ++k )
                {
                    sum += at( row, k ) * o.at( k, col );
                }
                r.at( row, col ) = sum;
            }
        }
        return r;
    }

    Vec4 operator*( const Vec4 & v ) const
    {
        return Vec4{
            at( 0, 0 ) * v.x + at( 0, 1 ) * v.y + at( 0, 2 ) * v.z + at( 0, 3 ) * v.w,
            at( 1, 0 ) * v.x + at( 1, 1 ) * v.y + at( 1, 2 ) * v.z + at( 1, 3 ) * v.w,
            at( 2, 0 ) * v.x + at( 2, 1 ) * v.y + at( 2, 2 ) * v.z + at( 2, 3 ) * v.w,
            at( 3, 0 ) * v.x + at( 3, 1 ) * v.y + at( 3, 2 ) * v.z + at( 3, 3 ) * v.w,
        };
    }

    Mat4 transpose() const
    {
        Mat4 r;
        for ( uint32_t row = 0; row < 4; ++row )
        {
            for ( uint32_t col = 0; col < 4; ++col )
            {
                r.at( row, col ) = at( col, row );
            }
        }
        return r;
    }

    // Общий обратный (Гаусс–Жордан с выбором главного элемента).
    // Вырожденная матрица — TM_ASSERT и возврат identity.
    Mat4 inverse() const
    {
        float aug[ 4 ][ 8 ];

        for ( uint32_t row = 0; row < 4; ++row )
        {
            for ( uint32_t col = 0; col < 4; ++col )
            {
                aug[ row ][ col ]     = at( row, col );
                aug[ row ][ col + 4 ] = ( row == col ) ? 1.0f : 0.0f;
            }
        }

        for ( uint32_t col = 0; col < 4; ++col )
        {
            // Выбор главного элемента по модулю.
            uint32_t pivot     = col;
            float    pivot_abs = aug[ col ][ col ] < 0.0f ? -aug[ col ][ col ] : aug[ col ][ col ];

            for ( uint32_t row = col + 1; row < 4; ++row )
            {
                const float v = aug[ row ][ col ] < 0.0f ? -aug[ row ][ col ] : aug[ row ][ col ];
                if ( v > pivot_abs )
                {
                    pivot     = row;
                    pivot_abs = v;
                }
            }

            if ( pivot_abs < 1.0e-12f )
            {
                TM_ASSERT( pivot_abs >= 1.0e-12f );
                return identity();
            }

            if ( pivot != col )
            {
                for ( uint32_t k = 0; k < 8; ++k )
                {
                    const float tmp     = aug[ col ][ k ];
                    aug[ col ][ k ]     = aug[ pivot ][ k ];
                    aug[ pivot ][ k ]   = tmp;
                }
            }

            const float inv = 1.0f / aug[ col ][ col ];
            for ( uint32_t k = 0; k < 8; ++k )
            {
                aug[ col ][ k ] *= inv;
            }

            for ( uint32_t row = 0; row < 4; ++row )
            {
                if ( row == col )
                {
                    continue;
                }
                const float factor = aug[ row ][ col ];
                for ( uint32_t k = 0; k < 8; ++k )
                {
                    aug[ row ][ k ] -= factor * aug[ col ][ k ];
                }
            }
        }

        Mat4 r;
        for ( uint32_t row = 0; row < 4; ++row )
        {
            for ( uint32_t col = 0; col < 4; ++col )
            {
                r.at( row, col ) = aug[ row ][ col + 4 ];
            }
        }
        return r;
    }

    // Ортографическая проекция (view-з near/far — ПОЛОЖИТЕЛЬНЫЕ расстояния).
    static Mat4 ortho( float left, float right,
                       float bottom, float top,
                       float zNear, float zFar )
    {
        Mat4 r;
        r.at( 0, 0 ) =  2.0f / ( right - left );
        r.at( 1, 1 ) =  2.0f / ( top - bottom );
        r.at( 2, 2 ) = -2.0f / ( zFar - zNear );
        r.at( 0, 3 ) = -( right + left ) / ( right - left );
        r.at( 1, 3 ) = -( top + bottom ) / ( top - bottom );
        r.at( 2, 3 ) = -( zFar + zNear ) / ( zFar - zNear );
        r.at( 3, 3 ) = 1.0f;
        return r;
    }

    // Перспективная проекция: fovYRad — вертикальный угол обзора в радианах.
    static Mat4 perspective( float fovYRad, float aspect, float zNear, float zFar )
    {
        const float tan_half = __builtin_tanf( fovYRad * 0.5f );

        Mat4 r;
        r.at( 0, 0 ) = 1.0f / ( aspect * tan_half );
        r.at( 1, 1 ) = 1.0f / tan_half;
        r.at( 2, 2 ) = -( zFar + zNear ) / ( zFar - zNear );
        r.at( 3, 2 ) = -1.0f;
        r.at( 2, 3 ) = -( 2.0f * zFar * zNear ) / ( zFar - zNear );
        return r;
    }

    // Видовая матрица камеры: eye — позиция, center — точка взгляда, up — «вверх».
    static Mat4 lookAt( const Vec3 & eye, const Vec3 & center, const Vec3 & up )
    {
        const Vec3 f = ( center - eye ).normalize();          // вперёд
        const Vec3 s = f.cross( up ).normalize();             // вправо
        const Vec3 u = s.cross( f );                          // настоящий up

        Mat4 r = identity();
        r.at( 0, 0 ) = s.x;
        r.at( 0, 1 ) = s.y;
        r.at( 0, 2 ) = s.z;
        r.at( 1, 0 ) = u.x;
        r.at( 1, 1 ) = u.y;
        r.at( 1, 2 ) = u.z;
        r.at( 2, 0 ) = -f.x;
        r.at( 2, 1 ) = -f.y;
        r.at( 2, 2 ) = -f.z;
        r.at( 0, 3 ) = -s.dot( eye );
        r.at( 1, 3 ) = -u.dot( eye );
        r.at( 2, 3 ) =  f.dot( eye );
        return r;
    }
};

} // namespace tml::math

#endif // ENGINE_MATH_MAT4_HH
