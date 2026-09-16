#include "testFramework.h"

#include "tml/core/handle.hh"
#include "tml/core/log.hh"
#include "tml/core/pool.hh"

#include <string>

namespace
{

struct TestParticle
{
    float x = 0.0f;

    TestParticle() = default;
    explicit TestParticle( float px ) : x( px ) {}
};

} // anonymous namespace

TM_TEST( handleDefaults )
{
    tml::Handle32 h;
    TM_CHECK( !h.isValid() );
    TM_CHECK( h.index == tml::Handle32::INVALID_INDEX );

    tml::Handle32 a;
    a.index = 3;
    a.gen   = 1;

    tml::Handle32 b;
    b.index = 3;
    b.gen   = 1;

    TM_CHECK( a.isValid() );
    TM_CHECK( a == b );
    TM_CHECK( !( a != b ) );

    b.gen = 2;
    TM_CHECK( a != b );
}

TM_TEST( handleTagsAreDistinctTypes )
{
    // Разные теги — разные типы: проверяем, что оба создаются и работают.
    tml::Handle< struct BodyTag >  body;
    tml::Handle< struct AssetTag > asset;

    body.index  = 1;
    asset.index = 1;

    TM_CHECK( body.index == asset.index );   // поля сравнить можно...
    // ...а сами хэндлы разных типов между собой не сравнимы — это проверяется
    // тем, что строка ниже НЕ скомпилировалась бы:
    //     body == asset;   // ошибка компиляции — и это хорошо
}

TM_TEST( poolCreateAndGet )
{
    tml::Pool< TestParticle > pool;

    tml::Handle< TestParticle > a = pool.create( TestParticle{ 1.0f } );
    tml::Handle< TestParticle > b = pool.create( TestParticle{ 2.0f } );
    tml::Handle< TestParticle > c = pool.create( TestParticle{ 3.0f } );

    TM_CHECK( pool.size() == 3 );

    TM_REQUIRE( pool.tryGet( a ) != nullptr );
    TM_REQUIRE( pool.tryGet( b ) != nullptr );
    TM_REQUIRE( pool.tryGet( c ) != nullptr );

    TM_CHECK( pool.tryGet( a )->x == 1.0f );
    TM_CHECK( pool.tryGet( b )->x == 2.0f );
    TM_CHECK( pool.tryGet( c )->x == 3.0f );

    // Запись через хэндл.
    pool.tryGet( b )->x = 20.0f;
    TM_CHECK( pool.tryGet( b )->x == 20.0f );
}

TM_TEST( poolStaleHandleAfterDestroy )
{
    tml::Pool< TestParticle > pool;

    tml::Handle< TestParticle > a = pool.create( TestParticle{ 1.0f } );
    tml::Handle< TestParticle > b = pool.create( TestParticle{ 2.0f } );

    pool.destroy( a );
    TM_CHECK( pool.size() == 1 );
    TM_CHECK( pool.tryGet( a ) == nullptr );
    TM_REQUIRE( pool.tryGet( b ) != nullptr );
    TM_CHECK( pool.tryGet( b )->x == 2.0f );

    // Новый create переиспользует слот с инкрементом поколения:
    // старый хэндл остаётся невалидным.
    tml::Handle< TestParticle > c = pool.create( TestParticle{ 3.0f } );
    TM_CHECK( c.index == a.index );
    TM_CHECK( c.gen == a.gen + 1 );
    TM_CHECK( pool.tryGet( a ) == nullptr );
    TM_REQUIRE( pool.tryGet( c ) != nullptr );
    TM_CHECK( pool.tryGet( c )->x == 3.0f );
}

TM_TEST( poolInvalidHandle )
{
    tml::Pool< TestParticle > pool;

    tml::Handle< TestParticle > invalid;
    TM_CHECK( pool.tryGet( invalid ) == nullptr );

    // Хэндл с индексом за пределами пула.
    tml::Handle< TestParticle > far;
    far.index = 100;
    far.gen   = 0;
    TM_CHECK( pool.tryGet( far ) == nullptr );
}

TM_TEST( poolSwapAndPopKeepsValues )
{
    tml::Pool< TestParticle > pool;

    tml::Handle< TestParticle > a = pool.create( TestParticle{ 1.0f } );
    tml::Handle< TestParticle > b = pool.create( TestParticle{ 2.0f } );
    tml::Handle< TestParticle > c = pool.create( TestParticle{ 3.0f } );

    // Удаляем из середины: элемент c переезжает на место a в плотном массиве,
    // но его хэндл обязан остаться валидным.
    pool.destroy( a );

    TM_REQUIRE( pool.tryGet( b ) != nullptr );
    TM_REQUIRE( pool.tryGet( c ) != nullptr );
    TM_CHECK( pool.tryGet( b )->x == 2.0f );
    TM_CHECK( pool.tryGet( c )->x == 3.0f );
    TM_CHECK( pool.size() == 2 );
}

TM_TEST( poolForEach )
{
    tml::Pool< TestParticle > pool;

    pool.create( TestParticle{ 1.0f } );
    pool.create( TestParticle{ 2.0f } );
    pool.create( TestParticle{ 4.0f } );

    float sum = 0.0f;
    pool.forEach( [ &sum ]( TestParticle & p ) { sum += p.x; } );
    TM_CHECK( sum == 7.0f );

    const tml::Pool< TestParticle > & const_pool = pool;
    float const_sum = 0.0f;
    const_pool.forEach( [ &const_sum ]( const TestParticle & p ) { const_sum += p.x; } );
    TM_CHECK( const_sum == 7.0f );
}

TM_TEST( poolClear )
{
    tml::Pool< TestParticle > pool;

    tml::Handle< TestParticle > a = pool.create( TestParticle{ 1.0f } );
    tml::Handle< TestParticle > b = pool.create( TestParticle{ 2.0f } );

    pool.clear();
    TM_CHECK( pool.size() == 0 );
    TM_CHECK( pool.tryGet( a ) == nullptr );
    TM_CHECK( pool.tryGet( b ) == nullptr );

    // После clear пул работает дальше, а старые хэндлы не воскресают.
    tml::Handle< TestParticle > c = pool.create( TestParticle{ 5.0f } );
    tml::Handle< TestParticle > d = pool.create( TestParticle{ 6.0f } );
    TM_CHECK( pool.size() == 2 );
    TM_CHECK( pool.tryGet( a ) == nullptr );
    TM_REQUIRE( pool.tryGet( c ) != nullptr );
    TM_REQUIRE( pool.tryGet( d ) != nullptr );
}

TM_TEST( poolMovableType )
{
    tml::Pool< std::string > pool;

    tml::Handle< std::string > a = pool.create( std::string( "hello" ) );
    tml::Handle< std::string > b = pool.create( std::string( "world" ) );

    pool.destroy( a );   // swap-and-pop со string — проверка move-семантики

    TM_REQUIRE( pool.tryGet( b ) != nullptr );
    TM_CHECK( *pool.tryGet( b ) == "world" );
}

TM_TEST( logSmoke )
{
    // Функциональный smoke: вызовы не падают, фильтрация применяется.
    tml::log::setLevel( tml::LogLevel::Debug );
    TM_TRACE( "test", "trace-проверка (не видна при уровне Debug)" );
    TM_DEBUG( "test", "debug-проверка %d", 1 );

    // Переопределение уровня по каналу.
    tml::log::setLevel( "quiet", tml::LogLevel::Error );
    TM_INFO( "quiet", "это сообщение отфильтровано каналом" );
    TM_ERROR( "quiet", "а это — видно" );

    tml::log::setLevel( tml::LogLevel::Info );
    TM_INFO( "test", "smoke завершён" );

    // Если дошли сюда без падения — тест пройден.
    TM_CHECK( true );
}
