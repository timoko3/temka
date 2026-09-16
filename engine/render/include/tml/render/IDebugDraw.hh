#ifndef ENGINE_RENDER_IDEBUG_DRAW_HH
#define ENGINE_RENDER_IDEBUG_DRAW_HH

#include "tml/math/vec3.hh"
#include "tml/math/aabb.hh"
#include "tml/render/camera.hh"
#include "tml/render/color.hh"

#include <cstdint>

// Контракт отладочной отрисовки (architecture.md §5.6).
//
// Физика знает ТОЛЬКО этот интерфейс — трактат рендера §6.1.
// 2D — частный случай 3D с z = 0, отдельного IDebugDraw2D не существует.

namespace tml::rnd
{

class IDebugDraw
{
public:
    virtual ~IDebugDraw() = default;

    virtual void begin( const Camera & camera ) = 0;
    virtual void end()                            = 0;

    virtual void point( math::Vec3 p, Color c )                               = 0;
    virtual void line( math::Vec3 a, math::Vec3 b, Color c )                  = 0;
    virtual void polyline( const math::Vec3 * pts, std::uint32_t n,
                           Color c, bool closed )                             = 0;

    virtual void circle( math::Vec3 center, float radius, Color c )           = 0;
    virtual void aabb( const math::AABB & box, Color c )                      = 0;
    virtual void sphere( math::Vec3 center, float radius, Color c )           = 0;
    virtual void triad( math::Vec3 origin, float size )                       = 0;
    virtual void arrow( math::Vec3 from, math::Vec3 to, Color c )             = 0;
    virtual void text( math::Vec3 pos, const char * utf8, Color c )           = 0;

    virtual void pushLayer() {}
    virtual void popLayer()  {}
};

} // namespace tml::rnd

#endif // ENGINE_RENDER_IDEBUG_DRAW_HH
