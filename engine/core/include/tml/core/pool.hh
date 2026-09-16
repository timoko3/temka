#ifndef ENGINE_CORE_POOL_HH
#define ENGINE_CORE_POOL_HH

#include "tml/core/assert.hh"
#include "tml/core/handle.hh"

#include <cstdint>
#include <utility>
#include <vector>

// Пул с поколениями (architecture.md §5.1) — базовый строительный блок
// всех модулей движка.
//
// Устройство (sparse-подобная схема):
//   slots_       — разрежённый массив по индексу хэндла: поколение + позиция
//                  в плотном массиве (FREE_POS, если слот свободен);
//   dense_       — плотный массив живых значений (итерация без дыр);
//   denseToSlot_ — обратное отображение dense -> slot;
//   freeList_    — свободные слоты для переиспользования.
//
// destroy() делает swap-and-pop в плотном массиве и инкрементирует поколение
// слота: старый хэндл перестаёт проходить проверку gen (tryGet -> nullptr).

namespace tml
{

template < typename T, typename Tag = T >
class Pool
{
public:
    Handle< Tag > create( T && value )
    {
        uint32_t slot = 0;
        if ( !freeList_.empty() )
        {
            slot = freeList_.back();
            freeList_.pop_back();
        }
        else
        {
            slot = static_cast< uint32_t >( slots_.size() );
            slots_.push_back( Slot{} );
        }

        slots_[ slot ].densePos = static_cast< uint32_t >( dense_.size() );
        dense_.push_back( std::move( value ) );
        denseToSlot_.push_back( slot );

        Handle< Tag > h;
        h.index = slot;
        h.gen   = slots_[ slot ].gen;
        return h;
    }

    Handle< Tag > create( const T & value )
    {
        return create( T( value ) );
    }

    T * tryGet( Handle< Tag > h )
    {
        if ( h.index >= slots_.size() )
            return nullptr;

        const Slot & s = slots_[ h.index ];
        if ( s.gen != h.gen || s.densePos == FREE_POS )
            return nullptr;

        return &dense_[ s.densePos ];
    }

    const T * tryGet( Handle< Tag > h ) const
    {
        if ( h.index >= slots_.size() )
            return nullptr;

        const Slot & s = slots_[ h.index ];
        if ( s.gen != h.gen || s.densePos == FREE_POS )
            return nullptr;

        return &dense_[ s.densePos ];
    }

    void destroy( Handle< Tag > h )
    {
        T * value = tryGet( h );
        TM_ASSERT_MSG( value != nullptr, "Pool::destroy: невалидный или протухший хэндл" );

        Slot &         s    = slots_[ h.index ];
        const uint32_t pos  = s.densePos;
        const uint32_t last = static_cast< uint32_t >( dense_.size() - 1 );

        if ( pos != last )
        {
            dense_[ pos ]               = std::move( dense_[ last ] );
            const uint32_t moved_slot   = denseToSlot_[ last ];
            denseToSlot_[ pos ]         = moved_slot;
            slots_[ moved_slot ].densePos = pos;
        }
        dense_.pop_back();
        denseToSlot_.pop_back();

        s.densePos = FREE_POS;
        ++s.gen;
        freeList_.push_back( h.index );
    }

    // Удаляет все значения. Живым слотам инкрементируется поколение,
    // поэтому все старые хэндлы остаются невалидными.
    void clear()
    {
        dense_.clear();
        denseToSlot_.clear();

        for ( uint32_t i = 0; i < static_cast< uint32_t >( slots_.size() ); ++i )
        {
            Slot & s = slots_[ i ];
            if ( s.densePos != FREE_POS )
            {
                s.densePos = FREE_POS;
                ++s.gen;
                freeList_.push_back( i );
            }
        }
    }

    uint32_t size() const
    {
        return static_cast< uint32_t >( dense_.size() );
    }

    // Плотная итерация по живым значениям.
    template < typename F >
    void forEach( F && fn )
    {
        for ( T & value : dense_ )
        {
            fn( value );
        }
    }

    template < typename F >
    void forEach( F && fn ) const
    {
        for ( const T & value : dense_ )
        {
            fn( value );
        }
    }

private:
    static constexpr uint32_t FREE_POS = 0xFFFFFFFFu;

    struct Slot
    {
        uint32_t gen      = 0;
        uint32_t densePos = FREE_POS;
    };

    std::vector< Slot >    slots_;
    std::vector< T >       dense_;
    std::vector< uint32_t > denseToSlot_;
    std::vector< uint32_t > freeList_;
};

} // namespace tml

#endif /* ENGINE_CORE_POOL_HH */
