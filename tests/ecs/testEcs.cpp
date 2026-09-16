#include "testFramework.h"

#include "tml/ecs/ecs.hh"
#include "tml/ecs/entity.hh"

#include <string>

using namespace tml::ecs;

namespace
{

struct Position
{
    float x = 0.0f;
    float y = 0.0f;

    Position()                              = default;
    Position( float px, float py )          : x( px ), y( py ) {}
};

struct Velocity
{
    float dx = 0.0f;
    float dy = 0.0f;

    Velocity()                              = default;
    Velocity( float vx, float vy )          : dx( vx ), dy( vy ) {}
};

struct Tag
{
    int id = 0;

    Tag()                                   = default;
    explicit Tag( int i )                   : id( i ) {}
};

struct Health
{
    int hp = 100;
};

} // anonymous namespace

TM_TEST( entityLifecycle )
{
    Registry r;

    Entity a = r.create();
    Entity b = r.create();
    Entity c = r.create();

    TM_REQUIRE( r.valid( a ) );
    TM_REQUIRE( r.valid( b ) );
    TM_REQUIRE( r.valid( c ) );
    TM_CHECK( r.size() == 3 );

    r.destroy( b );
    TM_CHECK( !r.valid( b ) );
    TM_CHECK( r.isPendingDestroy( b ) );
    TM_CHECK( r.valid( a ) );
    TM_CHECK( r.valid( c ) );
    TM_CHECK( r.size() == 2 );

    r.processDeferred();
    TM_CHECK( !r.valid( b ) );
    TM_CHECK( !r.isPendingDestroy( b ) );

    Entity d = r.create();
    TM_CHECK( r.valid( d ) );
    TM_CHECK( entityIndex( d ) == entityIndex( b ) );
    TM_CHECK( entityGeneration( d ) == entityGeneration( b ) + 1 );
    TM_CHECK( !r.valid( b ) );
    TM_CHECK( r.size() == 3 );
}

TM_TEST( components )
{
    Registry r;
    Entity e = r.create();

    TM_CHECK( !r.has<Position>( e ) );
    TM_CHECK( r.tryGet<Position>( e ) == nullptr );

    Position & p = r.emplace<Position>( e, 1.0f, 2.0f );
    TM_CHECK( r.has<Position>( e ) );
    TM_CHECK( p.x == 1.0f );
    TM_CHECK( p.y == 2.0f );
    TM_CHECK( r.get<Position>( e ).x == 1.0f );

    // Повторный emplace не перезаписывает существующий компонент.
    r.emplace<Position>( e, 9.0f, 9.0f );
    TM_CHECK( r.get<Position>( e ).x == 1.0f );

    r.emplace<Velocity>( e, 0.5f, -0.5f );
    TM_CHECK( r.has<Position>( e ) );
    TM_CHECK( r.has<Velocity>( e ) );

    r.remove<Position>( e );
    TM_CHECK( !r.has<Position>( e ) );
    TM_CHECK( r.has<Velocity>( e ) );

    // Удаление уже отсутствующего компонента безопасно.
    r.remove<Position>( e );
    TM_CHECK( r.has<Velocity>( e ) );
    TM_CHECK( r.tryGet<Velocity>( e ) != nullptr );

    // Удаление компонента, который никогда не добавлялся.
    r.remove<Health>( e );
    TM_CHECK( r.tryGet<Health>( e ) == nullptr );
}

TM_TEST( getOrEmplace )
{
    Registry r;
    Entity e = r.create();

    Position & first = r.getOrEmplace<Position>( e );
    first.x = 7.0f;
    first.y = 8.0f;

    Position & second = r.getOrEmplace<Position>( e );
    TM_CHECK( second.x == 7.0f );
    TM_CHECK( second.y == 8.0f );
    TM_CHECK( &first == &second );

    TM_CHECK( r.pool<Position>().size() == 1 );
}

TM_TEST( swapAndPop )
{
    Registry r;
    Entity e0 = r.create();
    Entity e1 = r.create();
    Entity e2 = r.create();

    r.emplace<Tag>( e0, 0 );
    r.emplace<Tag>( e1, 1 );
    r.emplace<Tag>( e2, 2 );

    r.remove<Tag>( e1 );

    TM_CHECK( r.has<Tag>( e0 ) );
    TM_CHECK( !r.has<Tag>( e1 ) );
    TM_CHECK( r.has<Tag>( e2 ) );
    TM_CHECK( r.get<Tag>( e0 ).id == 0 );
    TM_CHECK( r.get<Tag>( e2 ).id == 2 );
}

TM_TEST( each )
{
    Registry r;
    Entity entities[6];

    for ( int i = 0; i < 6; ++i )
    {
        entities[ i ] = r.create();
        r.emplace<Position>( entities[ i ], static_cast<float>( i ), static_cast<float>( i ) );

        if ( i % 2 == 0 )
        {
            r.emplace<Velocity>( entities[ i ], 1.0f, 1.0f );
        }

        if ( i % 3 == 0 )
        {
            r.emplace<Tag>( entities[ i ], i );
        }
    }

    int count = 0;
    r.each<Position>( [&]( Entity, Position & ) { ++count; } );
    TM_CHECK( count == 6 );

    count = 0;
    float sum_x = 0.0f;
    r.each<Position>( [&]( Entity e, Position & p )
    {
        if ( r.has<Velocity>( e ) )
        {
            ++count;
            sum_x += p.x;
        }
    } );
    TM_CHECK( count == 3 );
    TM_CHECK( sum_x == 0.0f + 2.0f + 4.0f );

    count = 0;
    r.each<Position>( [&]( Entity e, Position & )
    {
        if ( r.has<Velocity>( e ) && r.has<Tag>( e ) )
        {
            ++count;
            TM_CHECK( r.get<Tag>( e ).id == 0 );
        }
    } );
    TM_CHECK( count == 1 );

    r.each<Position>( [&]( Entity e, Position & p )
    {
        if ( Velocity * v = r.tryGet<Velocity>( e ) )
        {
            p.x += v->dx;
            p.y += v->dy;
        }
    } );
    TM_CHECK( r.get<Position>( entities[ 0 ] ).x == 1.0f );
    TM_CHECK( r.get<Position>( entities[ 2 ] ).x == 3.0f );
    TM_CHECK( r.get<Position>( entities[ 1 ] ).x == 1.0f );

    count = 0;
    r.each<Health>( [&]( Entity, Health & ) { ++count; } );
    TM_CHECK( count == 0 );
}

TM_TEST( constEach )
{
    Registry r;
    Entity e = r.create();
    r.emplace<Position>( e, 3.0f, 4.0f );

    const Registry & cr = r;

    int count = 0;
    cr.each<Position>( [&]( Entity, const Position & p )
    {
        ++count;
        TM_CHECK( p.x == 3.0f );
        TM_CHECK( p.y == 4.0f );
    } );
    TM_CHECK( count == 1 );
}

TM_TEST( signals )
{
    Registry r;
    int constructed = 0;
    int destroyed   = 0;
    Entity last_constructed = 0;
    Entity last_destroyed   = 0;

    r.onConstruct<Position>( [&]( Entity e, Position & )
    {
        ++constructed;
        last_constructed = e;
    } );

    r.onDestroy<Position>( [&]( Entity e, Position & )
    {
        ++destroyed;
        last_destroyed = e;
    } );

    Entity e = r.create();
    r.emplace<Position>( e, 1.0f, 1.0f );
    TM_CHECK( constructed == 1 );
    TM_CHECK( destroyed == 0 );
    TM_CHECK( last_constructed == e );

    r.remove<Position>( e );
    TM_CHECK( destroyed == 1 );
    TM_CHECK( last_destroyed == e );

    r.emplace<Position>( e, 2.0f, 2.0f );
    TM_CHECK( constructed == 2 );

    r.destroy( e );
    TM_CHECK( destroyed == 1 );
    r.processDeferred();
    TM_CHECK( destroyed == 2 );
    TM_CHECK( last_destroyed == e );
}

TM_TEST( staleHandleAfterRecycle )
{
    Registry r;
    Entity a = r.create();
    r.emplace<Tag>( a, 42 );

    r.destroy( a );
    r.processDeferred();

    Entity b = r.create();
    r.emplace<Tag>( b, 7 );

    TM_CHECK( !r.valid( a ) );
    TM_CHECK( !r.has<Tag>( a ) );
    TM_CHECK( r.tryGet<Tag>( a ) == nullptr );
    TM_CHECK( r.has<Tag>( b ) );
    TM_CHECK( r.get<Tag>( b ).id == 7 );
    TM_CHECK( r.size() == 1 );
}

TM_TEST( deferredDestroy )
{
    Registry r;
    Entity a = r.create();
    r.emplace<Position>( a, 1.0f, 2.0f );

    r.destroy( a );
    TM_CHECK( !r.valid( a ) );
    TM_CHECK( r.has<Position>( a ) );
    TM_CHECK( r.size() == 0 );

    r.processDeferred();
    TM_CHECK( !r.has<Position>( a ) );
    TM_CHECK( r.tryGet<Position>( a ) == nullptr );
    TM_CHECK( r.size() == 0 );
}

TM_TEST( noRecycleBeforeProcessDeferred )
{
    Registry r;
    Entity a = r.create();
    r.emplace<Position>( a, 1.0f, 2.0f );
    r.destroy( a );

    Entity b = r.create();
    TM_CHECK( entityIndex( b ) != entityIndex( a ) );
    TM_CHECK( r.valid( b ) );
    TM_CHECK( !r.valid( a ) );
    TM_CHECK( r.isPendingDestroy( a ) );

    r.processDeferred();
    Entity c = r.create();
    TM_CHECK( entityIndex( c ) == entityIndex( a ) );
    TM_CHECK( entityGeneration( c ) == entityGeneration( a ) + 1 );
    TM_CHECK( !r.valid( a ) );
    TM_CHECK( r.valid( c ) );
}

TM_TEST( deferredComponents )
{
    Registry r;
    Entity e0 = r.create();
    Entity e1 = r.create();
    r.emplace<Position>( e0, 0.0f, 0.0f );
    r.emplace<Position>( e1, 1.0f, 1.0f );

    int count = 0;
    r.each<Position>( [&]( Entity e, Position & )
    {
        ++count;
        r.emplaceDeferred<Velocity>( e, 10.0f, 10.0f );
        r.removeDeferred<Position>( e );
    } );
    TM_CHECK( count == 2 );
    TM_CHECK( !r.has<Velocity>( e0 ) );
    TM_CHECK( r.has<Position>( e0 ) );

    r.processDeferred();
    TM_CHECK( r.has<Velocity>( e0 ) );
    TM_CHECK( r.has<Velocity>( e1 ) );
    TM_CHECK( !r.has<Position>( e0 ) );
    TM_CHECK( !r.has<Position>( e1 ) );
}

TM_TEST( deferredIgnoredForDestroyedEntity )
{
    Registry r;
    Entity e = r.create();
    r.emplace<Position>( e, 0.0f, 0.0f );

    r.destroy( e );
    r.emplaceDeferred<Velocity>( e, 1.0f, 1.0f );
    r.processDeferred();

    TM_CHECK( !r.valid( e ) );
    TM_CHECK( !r.has<Velocity>( e ) );
}

TM_TEST( poolAccess )
{
    Registry r;
    Entity e0 = r.create();
    Entity e1 = r.create();
    r.emplace<Position>( e0, 0.0f, 0.0f );
    r.emplace<Position>( e1, 1.0f, 1.0f );

    ComponentManager<Position> & pool = r.pool<Position>();
    TM_CHECK( pool.size() == 2 );
    TM_CHECK( pool.entities().size() == 2 );
    TM_CHECK( pool.components().size() == 2 );

    bool found_e0 = false;
    bool found_e1 = false;
    for ( Entity e : pool.entities() )
    {
        if ( e == e0 )
        {
            found_e0 = true;
        }
        if ( e == e1 )
        {
            found_e1 = true;
        }
    }
    TM_CHECK( found_e0 );
    TM_CHECK( found_e1 );
}

TM_TEST( constPoolAccess )
{
    Registry r;
    Entity e = r.create();
    r.emplace<Position>( e, 5.0f, 6.0f );

    const Registry & cr = r;
    const ComponentManager<Position> & pool = cr.pool<Position>();
    TM_CHECK( pool.size() == 1 );
    TM_CHECK( pool.components().front().x == 5.0f );
}

TM_TEST( clearComponent )
{
    Registry r;
    Entity e0 = r.create();
    Entity e1 = r.create();
    r.emplace<Position>( e0, 1.0f, 1.0f );
    r.emplace<Position>( e1, 2.0f, 2.0f );

    int destroyed = 0;
    r.onDestroy<Position>( [&]( Entity, Position & ) { ++destroyed; } );

    r.clear<Position>();
    TM_CHECK( destroyed == 2 );
    TM_CHECK( !r.has<Position>( e0 ) );
    TM_CHECK( !r.has<Position>( e1 ) );
    TM_CHECK( r.pool<Position>().size() == 0 );
}

TM_TEST( processDeferredIsIdempotent )
{
    Registry r;
    Entity e = r.create();
    r.emplace<Position>( e, 1.0f, 2.0f );

    r.destroy( e );
    r.processDeferred();
    r.processDeferred();
    r.processDeferred();

    TM_CHECK( !r.valid( e ) );
    TM_CHECK( !r.has<Position>( e ) );
    TM_CHECK( r.size() == 0 );
}
