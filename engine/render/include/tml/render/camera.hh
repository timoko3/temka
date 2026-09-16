#ifndef ENGINE_RENDER_CAMERA_HH
#define ENGINE_RENDER_CAMERA_HH

#include "tml/math/aabb.hh"
#include "tml/math/mat4.hh"
#include "tml/math/vec2.hh"
#include "tml/math/vec3.hh"

// Унифицированная камера (architecture.md §5.6): 2D и 3D отладочная графика
// используют одну и ту же структуру, чтобы не переписывать контракт на этапе 4.

namespace tml::rnd
{

struct Camera
{
    math::Mat4 viewProj;
    math::AABB frustumAABB;

    // Ортографическая 2D-камера.
    //   center        — мировые координаты центра экрана;
    //   halfHeight    — половина высоты видимой области в мировых единицах;
    //   aspectWtoH    — отношение ширины к высоте viewport'а.
    static Camera ortho2D( math::Vec2 center, float halfHeight, float aspectWtoH )
    {
        const float halfWidth = halfHeight * aspectWtoH;
        Camera cam;
        cam.viewProj = math::Mat4::ortho( center.x - halfWidth,
                                          center.x + halfWidth,
                                          center.y - halfHeight,
                                          center.y + halfHeight,
                                          -1.0f, 1.0f );
        cam.frustumAABB.expand( math::Vec3{ center.x - halfWidth, center.y - halfHeight, -1.0f } );
        cam.frustumAABB.expand( math::Vec3{ center.x + halfWidth,  center.y + halfHeight,  1.0f } );
        return cam;
    }

    // ЗАГЛУШКА перспективной камеры: понадобится на 3D-этапе.
    static Camera perspective( math::Vec3, math::Vec3, float, float, float, float )
    {
        Camera cam;
        cam.viewProj = math::Mat4::identity();
        return cam;
    }
};

} // namespace tml::rnd

#endif // ENGINE_RENDER_CAMERA_HH
