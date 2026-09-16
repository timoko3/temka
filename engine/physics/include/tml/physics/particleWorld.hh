#ifndef ENGINE_PHYSICS_PARTICLE_WORLD_HH
#define ENGINE_PHYSICS_PARTICLE_WORLD_HH

#include "tml/core/pool.hh"
#include "tml/physics/world2d.hh"

// Мир частиц (architecture.md §5.7, modulePlan §12): пока что минимальная
// заглушка — гравитация, semi-implicit Euler, отскок от пола, debugDraw.
//
// Расширяющие методы (addSpring, force fields) появятся на следующих итерациях.

namespace tml::phys
{

class ParticleWorld : public IWorld2D
{
public:
    ParticleWorld();

    BodyId createBody( const BodyDesc2D & desc ) override;
    void   destroyBody( BodyId id ) override;

    void setGravity( math::Vec2 g ) override;

    void step( float fixedDt ) override;

    math::Vec2 position( BodyId id ) const override;
    math::Vec2 interpolatedPosition( BodyId id, float alpha ) const override;
    void       setTransform( BodyId id, math::Vec2 pos, float angle ) override;

    void applyForce( BodyId id, math::Vec2 force ) override;
    void applyImpulse( BodyId id, math::Vec2 impulse ) override;

    bool raycast( const math::Vec2 & from, const math::Vec2 & dir,
                  float maxDist, RaycastHit2D & out ) const override;

    void debugDraw( rnd::IDebugDraw & dd ) const override;

    void setContactListener( IContactListener * listener ) override;

private:
    struct BodyData
    {
        math::Vec2 position;
        math::Vec2 prevPosition;
        math::Vec2 velocity;
        math::Vec2 acceleration;
        float      invMass     = 1.0f;
        float      restitution = 0.5f;
        float      radius      = 0.5f;
    };

    tml::Pool< BodyData, struct BodyTag > bodies_;
    math::Vec2                            gravity_;
    float                                 floorY_          = -5.0f;
    IContactListener *                    contactListener_ = nullptr;
};

} // namespace tml::phys

#endif // ENGINE_PHYSICS_PARTICLE_WORLD_HH
