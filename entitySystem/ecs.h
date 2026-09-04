#ifndef ECS_HH
#define ECS_HH

#include "entity.h"

#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

namespace ECS
{

/* Общий виртуальный класс для менеждера компонентов.
   Это необходимо, что бы можно было создавать массив из менеждеров компонентов */
class IPoolBase
{
public:
    virtual ~IPoolBase() = default;
    virtual void removeIfOwned( Entity e ) = 0;
    virtual void clear() = 0;
};


/* При регистрации менеждера ему выдается свой ID, по которому в дальнейшем можно будет определить тип компонента */
inline std::uint32_t nextComponentId()
{
    static std::uint32_t counter = 0;
    return counter++;
}


/* Для каждого менеджера создается своя функция и в ней храниться необходимое значение компонента */
template< typename Component >
std::uint32_t getComponentId()
{
    static std::uint32_t id = nextComponentId();
    return id;
}


template < typename Component >
class ComponentManager : public IPoolBase
{
    friend class Registry;

public:
    static constexpr std::uint32_t NO_POS = 0xFFFFFFFFu;

    /* добавление элемента в менеждер. Не надо пугаться ... Args,
       это сделано для того, что бы можно было просто указать при добавлении
       необходимые аргументы конструктора, и они автоматически подставились. */
    template< typename... Args >
    Component & emplace( Entity e, Args && ... args )
    {
        std::uint32_t idx = entityIndex( e );
        if ( idx >= sparse_.size() )
        {
            sparse_.resize( idx + 1, NO_POS );
        }
        if ( sparse_[ idx ] != NO_POS && entities_[ sparse_[ idx ] ] == e )
        {
            return components_[ sparse_[ idx ] ];
        }

        components_.emplace_back( std::forward< Args >( args )... );
        entities_.push_back( e );
        sparse_[ idx ] = static_cast< std::uint32_t >( entities_.size() - 1 );

        Component & created = components_.back();
        for ( auto & fn : onConstruct_ )
        {
            fn( e, created );
        }
        return created;
    }

    void remove( Entity e )
    {
        if ( !has( e ) )
        {
            return;
        }

        std::uint32_t idx = entityIndex( e );
        std::uint32_t pos = sparse_[ idx ];

        for ( auto & fn : onDestroy_ )
        {
            fn( e, components_[ pos ] );
        }

        std::uint32_t last = static_cast< std::uint32_t >( entities_.size() - 1 );
        if ( pos != last )
        {
            components_[ pos ] = std::move( components_[ last ] );
            entities_[ pos ] = entities_[ last ];
            sparse_[ entityIndex( entities_[ pos ] ) ] = pos;
        }
        components_.pop_back();
        entities_.pop_back();
        sparse_[ idx ] = NO_POS;
    }

    void removeIfOwned( Entity e ) override
    {
        remove( e );
    }

    void clear() override
    {
        for ( std::size_t i = 0; i < entities_.size(); ++i )
        {
            for ( auto & fn : onDestroy_ )
            {
                fn( entities_[ i ], components_[ i ] );
            }
        }
        components_.clear();
        entities_.clear();
        sparse_.clear();
    }

    bool has( Entity e ) const
    {
        std::uint32_t idx = entityIndex( e );
        return idx < sparse_.size()
            && sparse_[ idx ] != NO_POS
            && entities_[ sparse_[ idx ] ] == e;
    }

    Component & get( Entity e )
    {
        Component * c = tryGet( e );
        assert( c != nullptr );
        return *c;
    }

    const Component & get( Entity e ) const
    {
        const Component * c = tryGet( e );
        assert( c != nullptr );
        return *c;
    }

    Component * tryGet( Entity e )
    {
        if ( !has( e ) )
        {
            return nullptr;
        }
        return &components_[ sparse_[ entityIndex( e ) ] ];
    }

    const Component * tryGet( Entity e ) const
    {
        if ( !has( e ) )
        {
            return nullptr;
        }
        return &components_[ sparse_[ entityIndex( e ) ] ];
    }

    std::size_t size() const
    {
        return entities_.size();
    }

    const std::vector< Entity > & entities() const
    {
        return entities_;
    }

    const std::vector< Component > & components() const
    {
        return components_;
    }

    std::vector< Component > & components()
    {
        return components_;
    }

private:
/* NOTE:
   !!!!!!!!!!!!!!!!!!!!!!
   Это ОЧЕНЬ ВАЖНАЯ ХУЙНЯ
   В общем - эти вектора - по факту - функции,
   которые будут вызываться на КАЖДОЕ добавление или удаление объекта.
   По факту в большинстве случаев это можно не использовать, но например,
   если использовать готовую сторонную библиотеку для коллизий, то тела
   внутри буду храниться прямо внутри объектов этой библиотеки и что бы каждый раз
   не прописывать удаление объекта. можно воспользоваться этим колбеком.

   Если будем писать трассы и дебаг - очень удобная штука. Кидаем сюда логгер и все ахуенно */
    std::vector< std::function< void( Entity, Component & ) > > onConstruct_;
    std::vector< std::function< void( Entity, Component & ) > > onDestroy_;


/* Стандартный sparse-set */
    std::vector< std::uint32_t > sparse_;
    std::vector< Component > components_;
    std::vector< Entity > entities_;
};


class Registry
{
private:
    enum class LiveStatus : uint8_t { Alive = 1, MustDeleted = 2, Error = 0 };

public:
    Registry() = default;

    Entity create()
    {
        std::uint32_t idx = -1;
        if ( !freeList_.empty() )
        {
            idx = freeList_.back();
            freeList_.pop_back();
        }
        else
        {
            assert( generations_.size() < 0xFFFFFFFFu );
            idx = static_cast< std::uint32_t >( generations_.size() );
            generations_.push_back( 0 );
            alive_.push_back( LiveStatus::Error );
        }
        alive_[ idx ] = LiveStatus::Alive;
        ++aliveCount_;
        return makeEntity( idx, generations_[ idx ] );
    }

    /* NOTE:
       !!!!!
       Удаление объектов отложено
    */
    void destroy( Entity e )
    {
        assert( valid( e ) );
        std::uint32_t idx = entityIndex( e );
        alive_[ idx ] = LiveStatus::MustDeleted;
        deferredDestroy_.push_back( e );
        --aliveCount_;
    }

    /* Стандартные проверки, геттеры и сеттеры */
    bool valid( Entity e ) const
    {
        std::uint32_t idx = entityIndex( e );
        return idx < generations_.size()
            && alive_[ idx ] == LiveStatus::Alive
            && generations_[ idx ] == entityGeneration( e );
    }

    bool isPendingDestroy( Entity e ) const
    {
        std::uint32_t idx = entityIndex( e );
        return idx < generations_.size()
            && alive_[ idx ] == LiveStatus::MustDeleted
            && generations_[ idx ] == entityGeneration( e );
    }

    std::size_t size() const
    {
        return aliveCount_;
    }

    /* Конструктор для компонента выбирается по его типу.
       Аргументы просто передаются напрямую в конструктор */
    template< typename Component, typename... Args >
    Component & emplace( Entity e, Args && ... args )
    {
        assert( valid( e ) );
        return pool< Component >().emplace( e, std::forward< Args >( args )... );
    }

    template< typename Component >
    bool has( Entity e ) const
    {
        const ComponentManager< Component > * p = tryPool< Component >();
        return p != nullptr && p->has( e );
    }

    template< typename Component >
    Component & get( Entity e )
    {
        Component * c = tryGet< Component >( e );
        assert( c != nullptr );
        return *c;
    }

    template< typename Component >
    const Component & get( Entity e ) const
    {
        const Component * c = tryGet< Component >( e );
        assert( c != nullptr );
        return *c;
    }

    template< typename Component >
    Component * tryGet( Entity e )
    {
        ComponentManager< Component > * p = tryPool< Component >();
        return p != nullptr ? p->tryGet( e ) : nullptr;
    }

    template< typename Component >
    const Component * tryGet( Entity e ) const
    {
        const ComponentManager< Component > * p = tryPool< Component >();
        return p != nullptr ? p->tryGet( e ) : nullptr;
    }

    template< typename Component >
    void remove( Entity e )
    {
        ComponentManager< Component > * p = tryPool< Component >();
        if ( p != nullptr )
        {
            p->remove( e );
        }
    }

    /* NOTE:
       Тут происходит такая хуйня:
       Эта штука нужна, что бы отложить создание компонента на потом.
       Функция ебащится в лябда-функцию с аргументами и затем отложено все создается */
    template< typename Component, typename... Args >
    void emplaceDeferred( Entity e, Args && ... args )
    {
        auto captured = std::make_tuple( std::forward< Args >( args )... );
        deferredEmplaces_.push_back( [this, e, captured = std::move( captured )]() mutable
        {
            if ( !valid( e ) )
            {
                return;
            }
            std::apply( [this, e]( auto && ... unpacked )
            {
                pool< Component >().emplace( e, std::forward< decltype( unpacked ) >( unpacked )... );
            }, std::move( captured ) );
        } );
    }

    /* Аналогично отложенное удаление */
    template< typename Component >
    void removeDeferred( Entity e )
    {
        deferredRemoves_.push_back( [this, e]()
        {
            if ( !valid( e ) )
            {
                return;
            }
            pool< Component >().remove( e );
        } );
    }

    template< typename Component >
    void clear()
    {
        ComponentManager< Component > * p = tryPool< Component >();
        if ( p != nullptr )
        {
            p->clear();
        }
    }

    template< typename Component, typename Func >
    void each( Func && fn )
    {
        ComponentManager< Component > * p = tryPool< Component >();
        if ( p == nullptr )
        {
            return;
        }
        for ( Entity e : p->entities() )
        {
            if ( !valid( e ) )
            {
                continue;
            }
            if ( p->has( e ) )
            {
                fn( e, *p->tryGet( e ) );
            }
        }
    }

    /* NOTE:
       Это основное API для всех модулей - проход функцией по пулу компонентов */
    template< typename Component, typename Func >
    void each( Func && fn ) const
    {
        const ComponentManager< Component > * p = tryPool< Component >();
        if ( p == nullptr )
        {
            return;
        }
        for ( Entity e : p->entities() )
        {
            if ( !valid( e ) )
            {
                continue;
            }
            if ( p->has( e ) )
            {
                fn( e, *p->tryGet( e ) );
            }
        }
    }

    template< typename Component >
    ComponentManager< Component > & pool()
    {
        std::uint32_t id = getComponentId< Component >();
        if ( id >= pools_.size() )
        {
            pools_.resize( id + 1 );
        }
        if ( !pools_[ id ] )
        {
            pools_[ id ].reset( new ComponentManager< Component >() );
        }
        return *static_cast< ComponentManager< Component > * >( pools_[ id ].get() );
    }

    template< typename Component >
    const ComponentManager< Component > & pool() const
    {
        std::uint32_t id = getComponentId< Component >();
        assert( id < pools_.size() && pools_[ id ] );
        return *static_cast< const ComponentManager< Component > * >( pools_[ id ].get() );
    }

    template< typename Component >
    void onConstruct( std::function< void( Entity, Component & ) > fn )
    {
        pool< Component >().onConstruct_.push_back( std::move( fn ) );
    }

    template< typename Component >
    void onDestroy( std::function< void( Entity, Component & ) > fn )
    {
        pool< Component >().onDestroy_.push_back( std::move( fn ) );
    }

    void processDeferred()
    {
        for ( auto & cmd : deferredRemoves_ )
        {
            cmd();
        }
        deferredRemoves_.clear();

        for ( auto & cmd : deferredEmplaces_ )
        {
            cmd();
        }
        deferredEmplaces_.clear();

        for ( Entity e : deferredDestroy_ )
        {
            std::uint32_t idx = entityIndex( e );
            if ( alive_[ idx ] != LiveStatus::MustDeleted || generations_[ idx ] != entityGeneration( e ) )
            {
                continue;
            }
            for ( auto & poolPtr : pools_ )
            {
                if ( poolPtr )
                {
                    poolPtr->removeIfOwned( e );
                }
            }
            alive_[ idx ] = LiveStatus::Error;
            ++generations_[ idx ];
            freeList_.push_back( idx );
        }
        deferredDestroy_.clear();
    }

private:
    template< typename Component >
    ComponentManager< Component > * tryPool()
    {
        std::uint32_t id = getComponentId< Component >();
        if ( id >= pools_.size() || !pools_[ id ] )
        {
            return nullptr;
        }
        return static_cast< ComponentManager< Component > * >( pools_[ id ].get() );
    }

    template< typename Component >
    const ComponentManager< Component > * tryPool() const
    {
        std::uint32_t id = getComponentId< Component >();
        if ( id >= pools_.size() || !pools_[ id ] )
        {
            return nullptr;
        }
        return static_cast< const ComponentManager< Component > * >( pools_[ id ].get() );
    }

    std::vector< std::unique_ptr< IPoolBase > > pools_;
    std::vector< std::uint32_t > generations_;
    std::vector< LiveStatus > alive_;
    std::vector< std::uint32_t > freeList_;
    std::vector< Entity > deferredDestroy_;
    std::vector< std::function< void() > > deferredRemoves_;
    std::vector< std::function< void() > > deferredEmplaces_;
    std::size_t aliveCount_ = 0;
};

} // namespace ECS

#endif // ECS_HH
