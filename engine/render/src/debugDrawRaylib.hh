#ifndef DEBUG_DRAW_RAYLIB_HH
#define DEBUG_DRAW_RAYLIB_HH

#include "tml/render/IDebugDraw.hh"

#include "tml/math/vec2.hh"
#include "tml/math/vec3.hh"

// Частная реализация IDebugDraw поверх raylib. Живёт только внутри
// tml_render_raylib и не видна потребителям render_api.

namespace tml::rnd
{

class DebugDrawRaylib : public IDebugDraw
{
public:
    DebugDrawRaylib() = default;

    void begin( const Camera & camera ) override;
    void end() override;

    void point( math::Vec3 p, Color c ) override;
    void line( math::Vec3 a, math::Vec3 b, Color c ) override;
    void polyline( const math::Vec3 * pts, std::uint32_t n,
                   Color c, bool closed ) override;

    void circle( math::Vec3 center, float radius, Color c ) override;
    void aabb( const math::AABB & box, Color c ) override;
    void sphere( math::Vec3 center, float radius, Color c ) override;
    void triad( math::Vec3 origin, float size ) override;
    void arrow( math::Vec3 from, math::Vec3 to, Color c ) override;
    void text( math::Vec3 pos, const char * utf8, Color c ) override;

private:
    math::Vec2 worldToScreen( math::Vec3 p ) const;

    Camera camera_;
    float  pixelsPerUnit_ = 1.0f;
};

} // namespace tml::rnd

#endif // DEBUG_DRAW_RAYLIB_HH
