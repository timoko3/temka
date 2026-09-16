# Документация по Entity System

Глобальный экземпляр реестра обычно доступен через синглтон или передаётся по ссылке в системы.

```cpp
ECS::Registry& registry = GetRegistry();
```

---

## 1. Жизненный цикл сущностей (Entity)

### Создание сущности

```cpp
ECS::Entity myEntity = registry.create();
```

### Удаление сущности

> **Важно:** удаление сущности в `ecs.hh` выполняется с отсрочкой. Сущность будет реально уничтожена только после вызова `registry.processDeferred()`.

```cpp
registry.destroy( myEntity );
```

### Проверка состояния сущности

```cpp
if ( registry.valid( myEntity ) )
{
    // сущность жива и не является "мусором"
}

if ( registry.isPendingDestroy( myEntity ) )
{
    // сущность помечена на удаление, но processDeferred() ещё не вызывался
}
```

---

## 2. Работа с компонентами

### Добавление компонента

Метод `emplace` вызывает конструктор компонента. Аргументы передаются напрямую в конструктор:

```cpp
registry.emplace<Transform>( myEntity, 10.0f, 20.0f, 0.0f );
registry.emplace<Health>( myEntity, 100 );
```

### Проверка наличия компонента

```cpp
if ( registry.has<Health>( myEntity ) )
{
    // ...
}
```

### Получение компонента

```cpp
// Если компонент точно есть. Иначе — assert.
Transform& t = registry.get<Transform>( myEntity );
t.position.x += 1.0f;
```

### Безопасное получение компонента

```cpp
if ( Health* hp = registry.tryGet<Health>( myEntity ) )
{
    hp->current -= 10;
}
```

### Удаление компонента

```cpp
registry.remove<Health>( myEntity );
```

Сущность при этом остаётся "живой".

---

## 3. Системы и итерация

Для обхода всех сущностей с определённым компонентом используйте `each`. Этот метод автоматически пропускает сущности, помеченные на удаление.

### Пример: система движения

```cpp
registry.each<Transform>( []( ECS::Entity e, Transform& t )
{
    t.position.x += 1.0f * deltaTime;
} );
```

### Несколько компонентов

Если логика требует нескольких компонентов, проверяйте их внутри `each`:

```cpp
registry.each<Transform>( []( ECS::Entity e, Transform& t )
{
    if ( registry.has<Velocity>( e ) )
    {
        Velocity& v = registry.get<Velocity>( e );
        t.position += v.direction * v.speed;
    }
} );
```

---

## 4. Безопасные (отложенные) операции внутри циклов

Внутри `registry.each()` нельзя вызывать:

- `registry.emplace(...)` — немедленно изменяет пул компонентов и ломает итерацию.
- `registry.remove(...)` — то же самое.

`registry.destroy(...)` в `ecs.hh` **можно** вызывать внутри `each`, потому что он сам по себе отложенный.

Для безопасного добавления и удаления компонентов внутри циклов используйте deferred-версии:

```cpp
registry.each<Health>( []( ECS::Entity e, Health& hp )
{
    if ( hp.current <= 0 )
    {
        registry.destroy( e );                              // OK: destroy уже отложенный
        registry.removeDeferred<Transform>( e );            // отложенное удаление
        registry.emplaceDeferred<Explosion>( e, e.position ); // отложенное добавление
    }
} );
```

### Применение отложенных операций

Чтобы все отложенные операции вступили в силу, вызовите `processDeferred()`. Обычно это делается один раз за кадр — в конце `Update` или в начале следующего кадра.

```cpp
registry.processDeferred();
```

---

## 5. События (callbacks)

Можно подписаться на создание и удаление компонентов. Это удобно для инициализации и очистки внешних ресурсов: физики, звука, рендера.

### onConstruct

Вызывается сразу при `registry.emplace<C>(...)`.

```cpp
registry.onConstruct<Sound>( []( ECS::Entity e, Sound& s )
{
    AudioEngine::play( s.fileId );
} );
```

### onDestroy

Вызывается **перед** физическим удалением компонента из памяти.

```cpp
registry.onDestroy<Sound>( []( ECS::Entity e, Sound& s )
{
    AudioEngine::stop( s.fileId );
} );
```

---

## Краткая шпаргалка

| Задача | Метод |
|--------|-------|
| Создать сущность | `registry.create()` |
| Удалить сущность | `registry.destroy(e)` |
| Проверить, жива ли сущность | `registry.valid(e)` |
| Добавить компонент | `registry.emplace<C>(e, args...)` |
| Проверить компонент | `registry.has<C>(e)` |
| Получить компонент | `registry.get<C>(e)` |
| Безопасно получить компонент | `registry.tryGet<C>(e)` |
| Удалить компонент | `registry.remove<C>(e)` |
| Безопасно удалить внутри each | `registry.removeDeferred<C>(e)` |
| Безопасно добавить внутри each | `registry.emplaceDeferred<C>(e, args...)` |
| Применить отложенные операции | `registry.processDeferred()` |
| Обойти сущности с компонентом | `registry.each<C>(fn)` |
| Подписаться на создание | `registry.onConstruct<C>(fn)` |
| Подписаться на удаление | `registry.onDestroy<C>(fn)` |
