# Код-стайл temka

> Документ обязателен для всего нового кода. Изменения стайла — через обсуждение и правку этого файла, а не молча в коде.
> Формат: C++17, компиляция чистая при `-Wall -Wextra`.

---

## 1. Именование

| Сущность | Стиль | Пример |
|---|---|---|
| Типы: `class`, `struct`, `enum`, `using`, шаблонные параметры | **CamelCase** | `Registry`, `Pool`, `PoolState` |
| Методы и функции | snake_case | `try_get`, `entity_index`, `make_entity` |
| Поля внутри классов | camelCase + **`_` в конце** | `sparse_`, `aliveCount_`, `world_` |
| Локальные переменные и параметры | snake_case | `first_particle`, `sum_x` |
| Константы (`constexpr`, `const` в классах) | camelCase, начинается с `k` | `kNoPos`, `kDefaultGravity` |
| Макросы | UPPER_SNAKE_CASE | `ECS_H`, `CHECK` |
| Пространства имён | короткий lowercase (??) | `ecs`, `tml` |
| Файлы | snake или camel ( с заглавной буквы) case  | `ecs.h`, `SomeObjects.cpp` |

- Аббревиатуры в именах склоняются как слова: `XmlDoc`, а не `XMLDoc` (но `ECS` в тексте — ок).
- Имя должно говорить «что», а не «как»: `aliveCount_`, а не `counterOfNonDeleted_`.

## 2. Форматирование

### 2.1 Базовые правила

- Отступ — **4 пробела**, табы запрещены. ( хз, как будто похуй)
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

После **каждой** открывающей скобки (круглой `(` и фигурной `{`) ставится пробел; симметрично — перед закрывающей( опционально):

```cpp
someFunc( arg1, arg2 );
if ( cond)
struct BodyDesc2D { Vec2 position; float mass; };
enum class PoolState { Empty, Full };
auto p = std::make_unique< Pool< C>>( 16);
```

- Пустые скобки пишутся **без** пробелов: `size()`, `{}`.
- После ключевого слова управляющей конструкции — пробел: `if (`, `for (`, `while (`, `switch (`.
- Пробелы вокруг бинарных операторов: `a + b`, `x == y`; после запятой — есть, перед — нет; после `;` в `for ( ;; )` — есть.
- `template <typename C>` — с пробелом после `template`.
- Указатель/ссылка отделены пробелами: `const Entity * e`, `Pool< C> & pool`, `std::unique_ptr< IPoolBase> ptr`.

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

- Гварды повторения — классические, `ИМЯ_ФАЙЛА_H` (не `#pragma once`) ( по мне похуй):

```cpp
#ifndef ECS_H
#define ECS_H
...
#endif // ECS_H
```

- Порядок включений: свой заголовок → заголовки проекта (`"..."`) → стандартные (`<...>`); внутри групп — алфавит.
- `using namespace` в заголовках **запрещён** (утекает в чужие единицы трансляции). В `.cpp` — допустим.
- Заголовок должен включать всё, что использует сам; не полагаться на порядок включений.

## 4. Классы и структуры

- `struct` — для агрегатных данных без инвариантов (компоненты ECS, desc-структуры, POD).
- `class` — при наличии инвариантов; все поля — `private` с `_` на конце.
- Порядок секций: `public` (интерфейс) → `protected` → `private` (данные внизу).
- Виртуальные переопределения — всегда с `override`; класс-лист без наследников — `final`.
- Конструктор с одним аргументом — `explicit` (кроме случаев копирования/инициализации).
- По умолчанию члены инициализируются при объявлении: `std::size_t aliveCount_ = 0;`.

## 5. Языковые правила

- `const`/`constexpr` везде, где уместно; методы-наблюдатели — `const`.
- Фиксированная ширина целых в API: `std::uint32_t`, `std::size_t` — не «голый» `int` в интерфейсах.
- `assert`/`TM_ASSERT` — для внутренних инвариантов; договорённости контрактов (UB) — документировать в заголовке.
- Никаких магических чисел — именованные константы.
- Без исключений из внутренних модулей движка (ошибка — assert/код возврата); `std::function` и аллокации не размещать в hot-path без необходимости.
- Закомментированный код и debug-`printf` в коммитах не оставляем ( в мерждах точно, в своих ветках - как хотим).

## 6. Совместимость со старым кодом

- `physicsEngine/` написан до стайла (Allman уже есть, но `using namespace` в заголовке и др.). **Не рефакторим впрок**: при правке файла переформатируем его целиком под стайл.
- Новый код — только по стайлу; ревью проверяет стайл по этому документу.

## 7. Эталонный пример

```cpp
#ifndef BODY_POOL_H
#define BODY_POOL_H

#include <cstdint>
#include <vector>

#include "handle.h"

namespace tml
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
    explicit BodyPool( std::size_t capacity ) :
        capacity_( capacity )
    {
        slots_.reserve( capacity );
    }

    Handle< BodyTag> create( const BodyDesc2D & desc )
    {
        if ( freeList_.empty() )
        {
            return pushSlot( desc );
        }
        const auto index = freeList_.back();
        freeList_.pop_back();
        slots_[ index].desc = desc;
        return makeHandle( index, slots_[ index].generation );
    }

    const BodyDesc2D * tryGet( Handle< BodyTag> handle ) const
    {
        if ( handle.generation != generationOf( handle ) )
            return nullptr;
        return &slots_[ handle.index].desc;
    }

private:
    struct Slot
    {
        BodyDesc2D desc;
        std::uint32_t generation = 0;
        bool alive = false;
    };

    Handle< BodyTag> pushSlot( const BodyDesc2D & desc );
    std::uint32_t generationOf( Handle< BodyTag> handle ) const;

    std::vector< Slot> slots_;
    std::vector< std::uint32_t> freeList_;
    std::size_t capacity_ = 0;
};

} // namespace tml

#endif // BODY_POOL_H
```

---

## 8. Автоматизация (план)

- `.clang-format` в корне: `BasedOnStyle: LLVM`, `BreakBeforeBraces: Allman`, пробелы внутри скобок (`SpacesInParens`), отступ 4, колонка 100.
- Ревью не тратит время на форматирование — только на смысл; всё механическое должен проверять форматтер.
