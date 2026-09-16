#ifndef ENGINE_ECS_ENTITY_HH
#define ENGINE_ECS_ENTITY_HH

#include <cstdint>

// Идентификатор сущности (architecture.md §5.5): младшие 32 бита — индекс,
// старшие 32 — поколение. Обёртки позволяют разбирать Entity без знания
// внутреннего устройства Registry.

namespace tml::ecs
{

using Entity = std::uint64_t;

inline std::uint32_t entityIndex( Entity e )
{
    return static_cast< std::uint32_t >( e );
}

inline std::uint32_t entityGeneration( Entity e )
{
    return static_cast< std::uint32_t >( e >> 32 );
}

inline Entity makeEntity( std::uint32_t index, std::uint32_t generation )
{
    return ( static_cast< Entity >( generation ) << 32 ) | static_cast< Entity >( index );
}

} // namespace tml::ecs

#endif // ENGINE_ECS_ENTITY_HH
