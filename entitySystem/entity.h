#ifndef ENTITY_H
#define ENTITY_H

#include <cstdint>

namespace ECS
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

} // namespace ECS

#endif // ENTITY_H
