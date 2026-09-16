#include "tml/physics/particleWorld.hh"

#include "tml/core/assert.hh"
#include "tml/core/pool.hh"
#include "tml/math/vec3.hh"
#include "tml/render/color.hh"

namespace tml::phys
{

ParticleWorld::ParticleWorld()
    : gravity_( 0.0f, -9.81f )
{
}

BodyId ParticleWorld::createBody( const BodyDesc2D & desc )
{
    BodyData bd;
    bd.position     = desc.position;
    bd.prevPosition = desc.position;
    bd.velocity     = desc.velocity;
    bd.acceleration = math::Vec2{ 0.0f, 0.0f };
    bd.invMass      = desc.mass > 0.0f ? 1.0f / desc.mass : 0.0f;
    bd.restitution  = desc.restitution;
    bd.radius       = desc.radius;
    return bodies_.create( std::move( bd ) );
}

void ParticleWorld::destroyBody( BodyId id )
{
    bodies_.destroy( id );
}

void ParticleWorld::setGravity( math::Vec2 g )
{
    gravity_ = g;
}

void ParticleWorld::step( float fixedDt )
{
    bodies_.forEach( [this, fixedDt]( BodyData & b )
    {
        if ( b.invMass == 0.0f )
        {
            return;
        }

        b.acceleration = gravity_;
        b.velocity    += b.acceleration * fixedDt;

        b.prevPosition = b.position;
        b.position    += b.velocity * fixedDt;

        // Отскок от пола.
        if ( b.position.y < floorY_ )
        {
            b.position.y  = floorY_;
            b.velocity.y *= -b.restitution;
        }
    } );
}

math::Vec2 ParticleWorld::position( BodyId id ) const
{
    const BodyData * b = bodies_.tryGet( id );
    TM_ASSERT( b != nullptr );
    return b->position;
}

math::Vec2 ParticleWorld::interpolatedPosition( BodyId id, float alpha ) const
{
    const BodyData * b = bodies_.tryGet( id );
    TM_ASSERT( b != nullptr );
    return b->prevPosition + ( b->position - b->prevPosition ) * alpha;
}

void ParticleWorld::setTransform( BodyId id, math::Vec2 pos, float /*angle*/ )
{
    BodyData * b = bodies_.tryGet( id );
    TM_ASSERT( b != nullptr );
    b->position     = pos;
    b->prevPosition = pos;
}

void ParticleWorld::applyForce( BodyId /*id*/, math::Vec2 /*force*/ )
{
    // ЗАГЛУШКА: на этом этапе силы не накапливаются, только гравитация.
}

void ParticleWorld::applyImpulse( BodyId /*id*/, math::Vec2 /*impulse*/ )
{
    // ЗАГЛУШКА.
}

bool ParticleWorld::raycast( const math::Vec2 & /*from*/, const math::Vec2 & /*dir*/,
                             float /*maxDist*/, RaycastHit2D & /*out*/ ) const
{
    // ЗАГЛУШКА: raycast не реализован на этом этапе.
    return false;
}

void ParticleWorld::debugDraw( rnd::IDebugDraw & dd ) const
{
    bodies_.forEach( [&dd]( const BodyData & b )
    {
        dd.circle( math::Vec3{ b.position.x, b.position.y, 0.0f },
                   b.radius,
                   rnd::Red() );
    } );
}

void ParticleWorld::setContactListener( IContactListener * listener )
{
    contactListener_ = listener;
}

} // namespace tml::phys
