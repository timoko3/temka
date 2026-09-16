#ifndef ENGINE_CORE_LOG_HH
#define ENGINE_CORE_LOG_HH

#include <cstdint>

// Логирование движка (architecture.md §5.1): уровни + каналы-теги.
//
// Уровень фильтрации задаётся глобально и переопределяется по каналу:
//     tml::log::setLevel( tml::LogLevel::Info );          // глобально
//     tml::log::setLevel( "phys", tml::LogLevel::Trace ); // только канал phys
//
// В коде используются только макросы TM_* — они подставляют уровень сами:
//     TM_INFO( "core", "window created: %ux%u", width, height );

namespace tml
{

enum class LogLevel : uint8_t { Trace, Debug, Info, Warn, Error, Fatal };

namespace log
{

void setLevel( LogLevel level );
void setLevel( const char * channel, LogLevel level );

void message( LogLevel level, const char * channel, const char * fmt, ... )
    __attribute__( ( format( printf, 3, 4 ) ) );

// Дополнительный вывод в файл (открывается на дозапись).
// nullptr — отключить файловый вывод. Возвращает false, если файл не открылся.
bool setFile( const char * path );

} // namespace log

} // namespace tml

#define TM_TRACE( ch, ... ) ::tml::log::message( ::tml::LogLevel::Trace, ch, __VA_ARGS__ )
#define TM_DEBUG( ch, ... ) ::tml::log::message( ::tml::LogLevel::Debug, ch, __VA_ARGS__ )
#define TM_INFO( ch,  ... ) ::tml::log::message( ::tml::LogLevel::Info,  ch, __VA_ARGS__ )
#define TM_WARN( ch, ... )  ::tml::log::message( ::tml::LogLevel::Warn,  ch, __VA_ARGS__ )
#define TM_ERROR( ch, ... ) ::tml::log::message( ::tml::LogLevel::Error, ch, __VA_ARGS__ )
#define TM_FATAL( ch, ... ) ::tml::log::message( ::tml::LogLevel::Fatal, ch, __VA_ARGS__ )

#endif /* ENGINE_CORE_LOG_HH */
