#include <cstdio>
#include <string>

#include "entitySystem/ecs.h"

using namespace ECS;

namespace
{

struct Position
{
    float x = 0.0f;
    float y = 0.0f;

    Position() = default;
    Position( float px, float py ) : x( px ), y( py ) {}
};

struct Velocity
{
    float dx = 0.0f;
    float dy = 0.0f;

    Velocity() = default;
    Velocity( float vx, float vy ) : dx( vx ), dy( vy ) {}
};

struct Tag
{
    int id = 0;

    Tag() = default;
    explicit Tag( int i ) : id( i ) {}
};

struct Health
{
    int hp = 100;
};

static int g_failures = 0;

static void reportFailure( const char * expression, const char * file, int line )
{
    ++g_failures;
    std::printf( "FAIL %s:%d: %s\n", file, line, expression );
}

static bool check( bool condition, const char * expression, const char * file, int line )
{
    if( !condition )
    {
        reportFailure( expression, file, line );
    }
    return condition;
}

#define CHECK( cond ) check( ( cond ), #cond, __FILE__, __LINE__ )

#define REQUIRE( cond )                                                   \
    do {                                                                  \
        if( !check( ( cond ), #cond, __FILE__, __LINE__ ) ) {             \
            return;                                                       \
        }                                                                 \
    } while( 0 )

static void testEntityLifecycle()
{
    Registry r;

    Entity a = r.create();
    Entity b = r.create();
    Entity c = r.create();

    REQUIRE( r.valid( a ) );
    REQUIRE( r.valid( b ) );
    REQUIRE( r.valid( c ) );
    REQUIRE( r.size() == 3 );

    r.destroy( b );
    REQUIRE( !r.valid( b ) );
    REQUIRE( r.isPendingDestroy( b ) );
    REQUIRE( r.valid( a ) );
    REQUIRE( r.valid( c ) );
    REQUIRE( r.size() == 2 );

    r.processDeferred();
    REQUIRE( !r.valid( b ) );
    REQUIRE( !r.isPendingDestroy( b ) );

    Entity d = r.create();
    REQUIRE( r.valid( d ) );
    REQUIRE( entityIndex( d ) == entityIndex( b ) );
    REQUIRE( entityGeneration( d ) == entityGeneration( b ) + 1 );
    REQUIRE( !r.valid( b ) );
    REQUIRE( r.size() == 3 );
}

static void testComponents()
{
    Registry r;
    Entity e = r.create();

    REQUIRE( !r.has<Position>( e ) );
    REQUIRE( r.tryGet<Position>( e ) == nullptr );

    Position & p = r.emplace<Position>( e, 1.0f, 2.0f );
    REQUIRE( r.has<Position>( e ) );
    REQUIRE( p.x == 1.0f );
    REQUIRE( p.y == 2.0f );
    REQUIRE( r.get<Position>( e ).x == 1.0f );

    // Повторный emplace не перезаписывает существующий компонент.
    r.emplace<Position>( e, 9.0f, 9.0f );
    REQUIRE( r.get<Position>( e ).x == 1.0f );

    r.emplace<Velocity>( e, 0.5f, -0.5f );
    REQUIRE( r.has<Position>( e ) );
    REQUIRE( r.has<Velocity>( e ) );

    r.remove<Position>( e );
    REQUIRE( !r.has<Position>( e ) );
    REQUIRE( r.has<Velocity>( e ) );

    // Удаление уже отсутствующего компонента безопасно.
    r.remove<Position>( e );
    REQUIRE( r.has<Velocity>( e ) );
    REQUIRE( r.tryGet<Velocity>( e ) != nullptr );

    // Удаление компонента, который никогда не добавлялся.
    r.remove<Health>( e );
    REQUIRE( r.tryGet<Health>( e ) == nullptr );
}

static void testSwapAndPop()
{
    Registry r;
    Entity e0 = r.create();
    Entity e1 = r.create();
    Entity e2 = r.create();

    r.emplace<Tag>( e0, 0 );
    r.emplace<Tag>( e1, 1 );
    r.emplace<Tag>( e2, 2 );

    r.remove<Tag>( e1 );

    REQUIRE( r.has<Tag>( e0 ) );
    REQUIRE( !r.has<Tag>( e1 ) );
    REQUIRE( r.has<Tag>( e2 ) );
    REQUIRE( r.get<Tag>( e0 ).id == 0 );
    REQUIRE( r.get<Tag>( e2 ).id == 2 );
}

static void testEach()
{
    Registry r;
    Entity entities[6];

    for( int i = 0; i < 6; ++i )
    {
        entities[ i ] = r.create();
        r.emplace<Position>( entities[ i ], static_cast<float>( i ), static_cast<float>( i ) );

        if( i % 2 == 0 )
        {
            r.emplace<Velocity>( entities[ i ], 1.0f, 1.0f );
        }

        if( i % 3 == 0 )
        {
            r.emplace<Tag>( entities[ i ], i );
        }
    }

    int count = 0;
    r.each<Position>( [&]( Entity, Position & ) { ++count; } );
    REQUIRE( count == 6 );

    count = 0;
    float sumX = 0.0f;
    r.each<Position>( [&]( Entity e, Position & p )
    {
        if( r.has<Velocity>( e ) )
        {
            ++count;
            sumX += p.x;
        }
    } );
    REQUIRE( count == 3 );
    REQUIRE( sumX == 0.0f + 2.0f + 4.0f );

    count = 0;
    r.each<Position>( [&]( Entity e, Position & )
    {
        if( r.has<Velocity>( e ) && r.has<Tag>( e ) )
        {
            ++count;
            REQUIRE( r.get<Tag>( e ).id == 0 );
        }
    } );
    REQUIRE( count == 1 );

    r.each<Position>( [&]( Entity e, Position & p )
    {
        if( Velocity * v = r.tryGet<Velocity>( e ) )
        {
            p.x += v->dx;
            p.y += v->dy;
        }
    } );
    REQUIRE( r.get<Position>( entities[ 0 ] ).x == 1.0f );
    REQUIRE( r.get<Position>( entities[ 2 ] ).x == 3.0f );
    REQUIRE( r.get<Position>( entities[ 1 ] ).x == 1.0f );

    count = 0;
    r.each<Health>( [&]( Entity, Health & ) { ++count; } );
    REQUIRE( count == 0 );
}

static void testConstEach()
{
    Registry r;
    Entity e = r.create();
    r.emplace<Position>( e, 3.0f, 4.0f );

    const Registry & cr = r;

    int count = 0;
    cr.each<Position>( [&]( Entity, const Position & p )
    {
        ++count;
        REQUIRE( p.x == 3.0f );
        REQUIRE( p.y == 4.0f );
    } );
    REQUIRE( count == 1 );
}

static void testSignals()
{
    Registry r;
    int constructed = 0;
    int destroyed = 0;
    Entity lastConstructed = 0;
    Entity lastDestroyed = 0;

    r.onConstruct<Position>( [&]( Entity e, Position & )
    {
        ++constructed;
        lastConstructed = e;
    } );

    r.onDestroy<Position>( [&]( Entity e, Position & )
    {
        ++destroyed;
        lastDestroyed = e;
    } );

    Entity e = r.create();
    r.emplace<Position>( e, 1.0f, 1.0f );
    REQUIRE( constructed == 1 );
    REQUIRE( destroyed == 0 );
    REQUIRE( lastConstructed == e );

    r.remove<Position>( e );
    REQUIRE( destroyed == 1 );
    REQUIRE( lastDestroyed == e );

    r.emplace<Position>( e, 2.0f, 2.0f );
    REQUIRE( constructed == 2 );

    r.destroy( e );
    REQUIRE( destroyed == 1 );
    r.processDeferred();
    REQUIRE( destroyed == 2 );
    REQUIRE( lastDestroyed == e );
}

static void testStaleHandleAfterRecycle()
{
    Registry r;
    Entity a = r.create();
    r.emplace<Tag>( a, 42 );

    r.destroy( a );
    r.processDeferred();

    Entity b = r.create();
    r.emplace<Tag>( b, 7 );

    REQUIRE( !r.valid( a ) );
    REQUIRE( !r.has<Tag>( a ) );
    REQUIRE( r.tryGet<Tag>( a ) == nullptr );
    REQUIRE( r.has<Tag>( b ) );
    REQUIRE( r.get<Tag>( b ).id == 7 );
    REQUIRE( r.size() == 1 );
}

static void testDeferredDestroy()
{
    Registry r;
    Entity a = r.create();
    r.emplace<Position>( a, 1.0f, 2.0f );

    r.destroy( a );
    REQUIRE( !r.valid( a ) );
    REQUIRE( r.has<Position>( a ) );
    REQUIRE( r.size() == 0 );

    r.processDeferred();
    REQUIRE( !r.has<Position>( a ) );
    REQUIRE( r.tryGet<Position>( a ) == nullptr );
    REQUIRE( r.size() == 0 );
}

static void testNoRecycleBeforeProcessDeferred()
{
    Registry r;
    Entity a = r.create();
    r.emplace<Position>( a, 1.0f, 2.0f );
    r.destroy( a );

    Entity b = r.create();
    REQUIRE( entityIndex( b ) != entityIndex( a ) );
    REQUIRE( r.valid( b ) );
    REQUIRE( !r.valid( a ) );
    REQUIRE( r.isPendingDestroy( a ) );

    r.processDeferred();
    Entity c = r.create();
    REQUIRE( entityIndex( c ) == entityIndex( a ) );
    REQUIRE( entityGeneration( c ) == entityGeneration( a ) + 1 );
    REQUIRE( !r.valid( a ) );
    REQUIRE( r.valid( c ) );
}

static void testDeferredComponents()
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
    REQUIRE( count == 2 );
    REQUIRE( !r.has<Velocity>( e0 ) );
    REQUIRE( r.has<Position>( e0 ) );

    r.processDeferred();
    REQUIRE( r.has<Velocity>( e0 ) );
    REQUIRE( r.has<Velocity>( e1 ) );
    REQUIRE( !r.has<Position>( e0 ) );
    REQUIRE( !r.has<Position>( e1 ) );
}

static void testDeferredIgnoredForDestroyedEntity()
{
    Registry r;
    Entity e = r.create();
    r.emplace<Position>( e, 0.0f, 0.0f );

    r.destroy( e );
    r.emplaceDeferred<Velocity>( e, 1.0f, 1.0f );
    r.processDeferred();

    REQUIRE( !r.valid( e ) );
    REQUIRE( !r.has<Velocity>( e ) );
}

static void testPoolAccess()
{
    Registry r;
    Entity e0 = r.create();
    Entity e1 = r.create();
    r.emplace<Position>( e0, 0.0f, 0.0f );
    r.emplace<Position>( e1, 1.0f, 1.0f );

    ComponentManager<Position> & pool = r.pool<Position>();
    REQUIRE( pool.size() == 2 );
    REQUIRE( pool.entities().size() == 2 );
    REQUIRE( pool.components().size() == 2 );

    bool foundE0 = false;
    bool foundE1 = false;
    for( Entity e : pool.entities() )
    {
        if( e == e0 )
        {
            foundE0 = true;
        }
        if( e == e1 )
        {
            foundE1 = true;
        }
    }
    REQUIRE( foundE0 );
    REQUIRE( foundE1 );
}

static void testConstPoolAccess()
{
    Registry r;
    Entity e = r.create();
    r.emplace<Position>( e, 5.0f, 6.0f );

    const Registry & cr = r;
    const ComponentManager<Position> & pool = cr.pool<Position>();
    REQUIRE( pool.size() == 1 );
    REQUIRE( pool.components().front().x == 5.0f );
}

static void testClearComponent()
{
    Registry r;
    Entity e0 = r.create();
    Entity e1 = r.create();
    r.emplace<Position>( e0, 1.0f, 1.0f );
    r.emplace<Position>( e1, 2.0f, 2.0f );

    int destroyed = 0;
    r.onDestroy<Position>( [&]( Entity, Position & ) { ++destroyed; } );

    r.clear<Position>();
    REQUIRE( destroyed == 2 );
    REQUIRE( !r.has<Position>( e0 ) );
    REQUIRE( !r.has<Position>( e1 ) );
    REQUIRE( r.pool<Position>().size() == 0 );
}

static void testProcessDeferredIsIdempotent()
{
    Registry r;
    Entity e = r.create();
    r.emplace<Position>( e, 1.0f, 2.0f );

    r.destroy( e );
    r.processDeferred();
    r.processDeferred();
    r.processDeferred();

    REQUIRE( !r.valid( e ) );
    REQUIRE( !r.has<Position>( e ) );
    REQUIRE( r.size() == 0 );
}

} // anonymous namespace

int main()
{
    struct TestCase
    {
        const char * name;
        void ( *run )();
    };

    TestCase cases[] = {
        { "entityLifecycle", testEntityLifecycle },
        { "components", testComponents },
        { "swapAndPop", testSwapAndPop },
        { "each", testEach },
        { "constEach", testConstEach },
        { "signals", testSignals },
        { "staleHandleAfterRecycle", testStaleHandleAfterRecycle },
        { "deferredDestroy", testDeferredDestroy },
        { "noRecycleBeforeProcessDeferred", testNoRecycleBeforeProcessDeferred },
        { "deferredComponents", testDeferredComponents },
        { "deferredIgnoredForDestroyedEntity", testDeferredIgnoredForDestroyedEntity },
        { "poolAccess", testPoolAccess },
        { "constPoolAccess", testConstPoolAccess },
        { "clearComponent", testClearComponent },
        { "processDeferredIsIdempotent", testProcessDeferredIsIdempotent },
    };

    const std::size_t testCount = sizeof( cases ) / sizeof( cases[ 0 ] );
    for( std::size_t i = 0; i < testCount; ++i )
    {
        std::printf( "[ RUN  ] %s\n", cases[ i ].name );
        cases[ i ].run();
        std::printf( "[ DONE ] %s\n", cases[ i ].name );
    }

    if( g_failures == 0 )
    {
        std::printf( "ALL TESTS PASSED (%zu tests)\n", testCount );
        return 0;
    }

    std::printf( "%d CHECK(S) FAILED\n", g_failures );
    return 1;
}
