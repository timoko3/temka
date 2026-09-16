#ifndef ENGINE_CORE_HANDLE_HH
#define ENGINE_CORE_HANDLE_HH

#include <cstdint>

// Поколенческий хэндл (architecture.md §5.1): 8 байт, стабилен при
// реаллокациях внутри владельца. Через границы модулей передаются только
// хэндлы и POD-структуры, не сырые указатели (трактат данных, §6.3).
//
// Тег различает хэндлы разных сущностей на уровне типов:
//     using BodyId  = tml::Handle< struct BodyTag >;
//     using AssetId = tml::Handle< struct AssetTag >;
// Handle<BodyTag> и Handle<AssetTag> — разные типы, их не перепутать.

namespace tml
{

struct Handle32
{
    static constexpr uint32_t INVALID_INDEX = 0xFFFFFFFFu;

    uint32_t index = INVALID_INDEX;
    uint32_t gen   = 0;

    bool isValid() const
    {
        return index != INVALID_INDEX;
    }

    // C++17: defaulted operator== недоступен (появился в C++20), пишем руками.
    bool operator==( const Handle32 & other ) const
    {
        return index == other.index && gen == other.gen;
    }

    bool operator!=( const Handle32 & other ) const
    {
        return !( *this == other );
    }
};

template < typename Tag >
struct Handle : Handle32
{
};

} // namespace tml

#endif /* ENGINE_CORE_HANDLE_HH */
