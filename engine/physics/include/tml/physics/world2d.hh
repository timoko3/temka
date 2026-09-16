#ifndef ENGINE_PHYSICS_WORLD2D_HH
#define ENGINE_PHYSICS_WORLD2D_HH

#include "tml/core/handle.hh"
#include "tml/math/vec2.hh"
#include "tml/render/IDebugDraw.hh"

// Общий интерфейс 2D-мира (architecture.md §5.7).
//
// Эту форму зеркалит будущий IWorld3D. Через этот интерфейс сцена и
// приложения общаются с физикой, не зная реализации.

namespace tml::phys
{

using BodyId = Handle< struct BodyTag >;

struct BodyDesc2D
{
    math::Vec2 position{ 0.0f, 0.0f };
    float      angle           = 0.0f;
    math::Vec2 velocity{ 0.0f, 0.0f };
    float      angularVelocity = 0.0f;
    float      mass            = 1.0f;     // 0 = static
    float      restitution     = 0.5f;
    float      friction        = 0.3f;
    float      radius          = 0.5f;     // для отладочной отрисовки
};

struct RaycastHit2D
{
    math::Vec2 point;
    math::Vec2 normal;
    float      fraction = 0.0f;
};

struct ContactEvent2D
{
    BodyId bodyA;
    BodyId bodyB;
};

class IWorld2D
{
public:
    struct IContactListener
    {
        virtual ~IContactListener()                         = default;
        virtual void onContact( const ContactEvent2D & )    {}
    };

    virtual ~IWorld2D() = default;

    virtual BodyId createBody( const BodyDesc2D & desc )                    = 0;
    virtual void   destroyBody( BodyId id )                                 = 0;

    virtual void   setGravity( math::Vec2 g )                               = 0;

    virtual void   step( float fixedDt )                                    = 0;

    virtual math::Vec2 position( BodyId id ) const                          = 0;
    virtual math::Vec2 interpolatedPosition( BodyId id, float alpha ) const = 0;
    virtual void       setTransform( BodyId id, math::Vec2 pos, float angle ) = 0;

    virtual void   applyForce( BodyId id, math::Vec2 force )                = 0;
    virtual void   applyImpulse( BodyId id, math::Vec2 impulse )            = 0;

    virtual bool   raycast( const math::Vec2 & from, const math::Vec2 & dir,
                            float maxDist, RaycastHit2D & out ) const       = 0;

    virtual void   debugDraw( rnd::IDebugDraw & dd ) const                  = 0;

    virtual void   setContactListener( IContactListener * listener )        = 0;
};

} // namespace tml::phys

#endif // ENGINE_PHYSICS_WORLD2D_HH
