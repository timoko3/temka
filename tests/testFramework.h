#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <cstdio>
#include <vector>

// Мини-фреймворк тестов (architecture.md §10): ноль внешних зависимостей,
// авторегистрация через статический объект.
//
// Использование в любом tests/**/*.cpp:
//     TM_TEST( poolGenerations )
//     {
//         TM_CHECK( pool.size() == 0 );
//         ...
//     }
//
// TM_CHECK  — фиксирует провал и продолжает тест;
// TM_REQUIRE — фиксирует провал и прерывает тест (для инвариантов, без
//              которых дальше проверять бессмысленно).

namespace tml::test
{

struct TestCase
{
    const char * name;
    void ( * run )();
};

inline std::vector< TestCase > & registry()
{
    static std::vector< TestCase > tests;
    return tests;
}

// Объект регистратора: создаётся при статической инициализации каждого
// TM_TEST и заносит тест в общий реестр.
struct Registrar
{
    Registrar( const char * name, void ( * fn )() )
    {
        registry().push_back( TestCase{ name, fn } );
    }
};

inline int & failureCount()
{
    static int failures = 0;
    return failures;
}

inline void reportFailure( const char * expression, const char * file, int line )
{
    ++failureCount();
    std::printf( "  CHECK FAILED  %s:%d: %s\n", file, line, expression );
}

inline int runAll()
{
    int failed_tests = 0;

    for ( const TestCase & tc : registry() )
    {
        std::printf( "[ RUN  ] %s\n", tc.name );

        const int before = failureCount();
        tc.run();

        if ( failureCount() == before )
        {
            std::printf( "[ OK   ] %s\n", tc.name );
        }
        else
        {
            std::printf( "[ FAIL ] %s (%d провалов)\n", tc.name, failureCount() - before );
            ++failed_tests;
        }
    }

    std::printf( "----------------------------------------\n" );
    std::printf( "Тестов: %zu, провалено: %d, проверок провалено: %d\n",
                 registry().size(), failed_tests, failureCount() );

    return failed_tests == 0 ? 0 : 1;
}

} // namespace tml::test

#define TM_TEST( test_name )                                                     \
    static void test_name();                                                     \
    static const ::tml::test::Registrar registrar_##test_name( #test_name,       \
                                                               &test_name );     \
    static void test_name()

#define TM_CHECK( cond )                                                         \
    do                                                                           \
    {                                                                            \
        if ( !( cond ) )                                                         \
            ::tml::test::reportFailure( #cond, __FILE__, __LINE__ );             \
    } while ( 0 )

#define TM_REQUIRE( cond )                                                       \
    do                                                                           \
    {                                                                            \
        if ( !( cond ) )                                                         \
        {                                                                        \
            ::tml::test::reportFailure( #cond, __FILE__, __LINE__ );             \
            return;                                                              \
        }                                                                        \
    } while ( 0 )

#endif /* TEST_FRAMEWORK_H */
