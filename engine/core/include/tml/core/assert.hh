#ifndef ENGINE_CORE_ASSERT_HH
#define ENGINE_CORE_ASSERT_HH

// Assert движка (architecture.md §5.1, modulePlan §1).
//
// Политика переключается сборкой:
//   - Debug (без NDEBUG): активен — пишет в лог (уровень Fatal) и делает abort;
//   - Release (NDEBUG):   выключен, нулевая стоимость;
//   - заставить работать в Release: определить TML_ASSERTS_FORCE_ON
//     (например, -DTML_ASSERTS_FORCE_ON в CXXFLAGS).

namespace tml
{

// Точка входа для макросов; напрямую не вызывать.
// fmt == nullptr — сообщения нет (случай TM_ASSERT).
[[noreturn]] void assertFail( const char * expr,
                              const char * file,
                              int          line,
                              const char * fmt = nullptr,
                              ... );

} // namespace tml

#if defined( NDEBUG ) && !defined( TML_ASSERTS_FORCE_ON )

#define TM_ASSERT( cond )          ( (void)0 )
#define TM_ASSERT_MSG( cond, ... ) ( (void)0 )

#else

#define TM_ASSERT( cond )                                                \
    do                                                                   \
    {                                                                    \
        if ( !( cond ) )                                                 \
            ::tml::assertFail( #cond, __FILE__, __LINE__ );              \
    } while ( 0 )

#define TM_ASSERT_MSG( cond, ... )                                       \
    do                                                                   \
    {                                                                    \
        if ( !( cond ) )                                                 \
            ::tml::assertFail( #cond, __FILE__, __LINE__, __VA_ARGS__ ); \
    } while ( 0 )

#endif

#endif // ENGINE_CORE_ASSERT_HH
