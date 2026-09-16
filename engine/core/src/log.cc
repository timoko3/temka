#include "tml/core/log.hh"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

namespace tml
{
namespace
{

// Всё состояние лога — в одном месте; никаких разрозненных глобалов.
struct ChannelLevel
{
    std::string channel;
    LogLevel    level;
};

struct LogState
{
    LogLevel                    globalLevel = LogLevel::Info;
    std::vector< ChannelLevel > channels;
    FILE *                      file = nullptr;
    std::mutex                  mutex;
};

LogState & state()
{
    // static внутри функции: ленивая инициализация, нет проблемы порядка
    // инициализации глобалов между единицами трансляции.
    static LogState s;
    return s;
}

const char * levelName( LogLevel level )
{
    switch ( level )
    {
    case LogLevel::Trace:
        return "TRACE";
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warn:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Fatal:
        return "FATAL";
    default:
        return "?";
    }
}

LogLevel effectiveLevel( const LogState & s, const char * channel )
{
    for ( const ChannelLevel & c : s.channels )
    {
        if ( c.channel == channel )
        {
            return c.level;
        }
    }
    return s.globalLevel;
}

} // anonymous namespace

namespace log
{

void setLevel( LogLevel level )
{
    std::lock_guard< std::mutex > lock( state().mutex );
    state().globalLevel = level;
}

void setLevel( const char * channel, LogLevel level )
{
    std::lock_guard< std::mutex > lock( state().mutex );

    for ( ChannelLevel & c : state().channels )
    {
        if ( c.channel == channel )
        {
            c.level = level;
            return;
        }
    }
    state().channels.push_back( ChannelLevel{ channel, level } );
}

bool setFile( const char * path )
{
    std::lock_guard< std::mutex > lock( state().mutex );

    if ( state().file != nullptr )
    {
        std::fclose( state().file );
        state().file = nullptr;
    }

    if ( path == nullptr )
    {
        return true;
    }

    state().file = std::fopen( path, "a" );
    return state().file != nullptr;
}

void message( LogLevel level, const char * channel, const char * fmt, ... )
{
    std::lock_guard< std::mutex > lock( state().mutex );

    if ( channel == nullptr )
    {
        channel = "default";
    }

    if ( level < effectiveLevel( state(), channel ) )
    {
        return;
    }

    // Форматируем в стековый буфер; куча — только если строка не влезла
    // (не больше одного форматирования на вызов).
    char stack_buf[1024];

    va_list args;
    va_start( args, fmt );
    va_list args_copy;
    va_copy( args_copy, args );

    const int needed = std::vsnprintf( stack_buf, sizeof( stack_buf ), fmt, args );
    va_end( args );

    if ( needed < 0 )
    {
        va_end( args_copy );
        return;
    }

    std::string heap_buf;
    const char * text = stack_buf;

    if ( static_cast< size_t >( needed ) >= sizeof( stack_buf ) )
    {
        heap_buf.resize( static_cast< size_t >( needed ) + 1 );
        std::vsnprintf( heap_buf.data(), heap_buf.size(), fmt, args_copy );
        text = heap_buf.c_str();
    }
    va_end( args_copy );

    // Warn и ниже — в stdout, Error/Fatal — в stderr.
    FILE * out = ( level >= LogLevel::Error ) ? stderr : stdout;
    std::fprintf( out, "[%s][%s] %s\n", levelName( level ), channel, text );
    std::fflush( out );

    if ( state().file != nullptr )
    {
        std::fprintf( state().file, "[%s][%s] %s\n", levelName( level ), channel, text );
        std::fflush( state().file );
    }
}

} // namespace log

} // namespace tml
