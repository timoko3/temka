# Код-стайл temka

> Документ обязателен для всего нового кода. Изменения стайла — через обсуждение и правку этого файла, а не молча в коде.
> Формат: C++17, компиляция чистая при `-Wall -Wextra`.

---

## 1. Именование

| Сущность | Стиль | Пример |
|---|---|---|
| Типы: `class`, `struct`, `enum`, `using`, шаблонные параметры | **upper CamelCase** | `Registry`, `Pool`, `PoolState` |
| Методы и функции | lower camelCase | `tryGet`, `entityIndex`, `makeEntity` |
| Поля внутри классов | camelCase + **`_` в конце** | `sparse_`, `aliveCount_`, `world_` |
| Локальные переменные и параметры | snake_case | `first_particle`, `sum_x` |
| Константы (`constexpr`, `const` в классах) и макросы | UPPER_SNAKE_CASE | `NO_POS`, `DEFAULT_GRAVITY` |
| Пространства имён | короткий **upper CamelCase** | `ECS`, `TML` |
| Файлы | snake_case  | `ecs.h`, `some_objects.cpp` |

- Аббревиатуры в именах склоняются как слова: `XmlDoc`, а не `XMLDoc` (но `ECS` в тексте — ок).
- Имя должно говорить «что», а не «как»: `aliveCount_`, а не `counterOfNonDeleted_`.

## 2. Форматирование

### 2.1 Базовые правила

- Отступ — **4 пробела**.
- Кодировка UTF-8, переводы строк LF.

### 2.2 Фигурные скобки — стиль **Allman**

Открывающая скобка блока — на **новой строке**, закрывающая — на своей, обе на уровне оператора:

```cpp
void Registry::destroy( Entity e )
{
    for ( auto & kv : pools_ )
    {
        kv.second->removeIfOwned( e );
    }
}
```

- `else`, `catch`, `while` после `}` — на новой строке:

```cpp
if ( valid( e ) )
{
    remove( e );
}
else
{
    create( e );
}
```

- Скобки обязательны у **всех** блоков `if` / `else` / `for` / `while` — даже однострочных. Единственное исключение — единственный короткий `return`/`continue` на той же строке:

```cpp
if ( e == InvalidEntity )
    return;
```

### 2.3 Пробелы внутри скобок

После **каждой** открывающей скобки (круглой `(` и фигурной `{`) ставится пробел; симметрично — перед закрывающей:

```cpp
someFunc( arg1, arg2 );
if ( cond )
struct BodyDesc2D { Vec2 position; float mass; };
enum class PoolState { Empty, Full };
auto p = std::make_unique< Pool< C >>( 16);
```

- Пустые скобки пишутся **без** пробелов: `size()`, `{}`.
- После ключевого слова управляющей конструкции — пробел: `if (`, `for (`, `while (`, `switch (`.
- Пробелы вокруг бинарных операторов: `a + b`, `x == y`; после запятой — есть, перед — нет; после `;` в `for ( ;; )` — есть.
- `template <typename C >` —  `template` разделен пробелами.
- Указатель/ссылка отделены пробелами: `const Entity * e`, `Pool< C > & pool`, `std::unique_ptr< IPoolBase > ptr`.

### 2.4 Прочее

- `switch`: `case` на уровне `switch`, тело — с отступом; каждый `case` завершается `break`/`return`/`[[fallthrough]]`.
- Список инициализации конструктора: каждое поле — на своей строке, двоеточие после `()`.

```cpp
Pool( std::size_t capacity, PoolState state ) :
    capacity_( capacity ),
    state_( state )
{
}
```

- Один класс — один заголовок (мелкие тесно связанные типы можно группировать).

## 3. Заголовочные файлы и include

- Гварды повторения — классические, `ИМЯ_ФАЙЛА_H` (не `#pragma once`):

```cpp
#ifndef ECS_H
#define ECS_H
...
#endif // ECS_H
```

- Порядок включений: свой заголовок → заголовки проекта (`"..."`) → стандартные (`<...>`); внутри групп — алфавит.
- Заголовок должен включать всё, что использует сам; не полагаться на порядок включений.

## 4. Классы и структуры

- `struct` — для общедоступных данных (компоненты ECS, desc-структуры, POD).
- `class` — при объектов; стараемся сделать все поля — `private`.
- Порядок секций: `public` (интерфейс) → `protected` → `private` (данные внизу).
- Виртуальные переопределения — всегда с `override`; класс-лист без наследников — `final`.
- По умолчанию члены инициализируются при объявлении: `std::size_t aliveCount_ = 0;`.

## 5. Языковые правила

- `const`/`constexpr` везде, где уместно; методы-наблюдатели — `const`.
- Фиксированная ширина целых в API: `std::uint32_t`, `size_t` — не «голый» `int` в интерфейсах.
- `assert`/`TM_ASSERT` — для внутренних инвариантов; договорённости контрактов (UB) — документировать в заголовке.
- Никаких магических чисел — именованные константы.
- Без исключений из внутренних модулей движка (ошибка — assert/код возврата);
- Закомментированный код и debug-`printf` в коммитах не оставляем ( в мерждах точно, в своих ветках - как хотим).

## 6. Эталонный пример

```cpp
#ifndef BODY_POOL_H
#define BODY_POOL_H

#include <cstdint>
#include <vector>

#include "handle.h"

namespace TML
{

enum class BodyKind
{
    Static,
    Dynamic,
};

struct BodyDesc2D
{
    Vec2 position = { 0.0f, 0.0f };
    float mass = 1.0f;
    BodyKind kind = BodyKind::Dynamic;
};

class BodyPool final
{
public:
    BodyPool( std::size_t capacity ) :
        capacity_( capacity )
    {
        slots_.reserve( capacity );
    }

    Handle< BodyTag > create( const BodyDesc2D & desc )
    {
        if ( freeList_.empty() )
        {
            return pushSlot( desc );
        }
        const auto index = freeList_.back();
        freeList_.pop_back();
        slots_[ index ].desc = desc;
        return makeHandle( index, slots_[ index ].generation_ );
    }

    const BodyDesc2D * tryGet( Handle< BodyTag > handle ) const
    {
        if ( handle.generation_ != generationOf( handle ) )
            return nullptr;
        return &slots_[ handle.index ].desc;
    }

private:
    struct Slot
    {
        BodyDesc2D desc_;
        std::uint32_t generation_ = 0;
        bool alive_ = false;
    };

    Handle< BodyTag> pushSlot( const BodyDesc2D & desc );
    std::uint32_t generationOf( Handle< BodyTag > handle ) const;

    std::vector< Slot > slots_;
    std::vector< std::uint32_t > freeList_;
    std::size_t capacity_ = 0;
};

} // namespace tml

#endif // BODY_POOL_H
```
---
