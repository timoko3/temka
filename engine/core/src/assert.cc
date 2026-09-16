#include "tml/core/assert.hh"

#include "tml/core/log.hh"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace tml
{

void assertFail( const char * expr, const char * file, int line, const char * fmt, ... )
{
    char user_msg[512];
    user_msg[0] = '\0';

    if ( fmt != nullptr )
    {
        va_list args;
        va_start( args, fmt );
        std::vsnprintf( user_msg, sizeof( user_msg ), fmt, args );
        va_end( args );
    }

    // Сообщение пользователя передаём через %s — его собственные
    // '%'-последовательности не интерпретируются.
    log::message( LogLevel::Fatal, "assert", "%s:%d: проверка провалена: %s%s%s",
                  file, line, expr,
                  fmt != nullptr ? " — " : "", user_msg );

    std::abort();
}

} // namespace tml
