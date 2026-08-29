# Архитектура temka.exe

> Документ фиксирует модульную структуру, слои и межмодульные API движка.
> Основа для параллельной работы треков (рендер / физика / инструменты): сначала подписываемся на этот документ, потом пишем код.
> Стандарт: **C++17**. Код подсистем пишем свой, по архитектуре проверенных библиотек (entt, enkiTS, spdlog, GLM), не используя их код.

---

## 1. Принципы

1. **Слоистость.** Зависимости смотрят только вниз. Модуль с этажа N знает о модулях этажа ≤ N−1, никогда выше.
2. **Контракт через интерфейс, а не через реализацию.** Физика не знает про SFML/OpenGL. Рендер не знает про массы и импульсы. Сцена связывает их через API-тиры.
3. **Расширение = новый модуль, а не правка старого.** Новый бэкенд рендера (GL, Vulkan), новый вид физики (soft body, SPH), редактор — каждый добавляется как отдельная единица сборки, реализующая существующий (или новый, но изолированный) интерфейс.
4. **Хэндлы вместо указателей.** Через границы модулей передаются `Handle<T>` (generation-based) и POD-структуры, не сырые указатели. Внутри модуля — что угодно.
5. **Детерминизм и фиксированный шаг.** Физика считается только на fixed timestep. Ввод, симуляция и рендер разделены. Это же открывает дорогу к фиче «подбор параметров по видео» (реплей сцены).
6. **Единицы и оси (договориться один раз):** метры, килограммы, секунды; правая система координат; 3D — Y-up (совместимо с glTF); 2D — x вправо, y вверх.
7. **C++17-идиоматика:** `std::optional`, `string_view`, `if constexpr`, шаблоны без концептов; логирование printf-style (в C++17 нет `std::format`).

---

## 2. Слои и направление зависимостей

```
┌──────────────────────────────────────────────────────────────┐
│  L6  Приложения: playground, editor, (буд.) runtime          │
├──────────────────────────────────────────────────────────────┤
│  L5  Фасад движка: Engine, game loop, Layer stack            │
├──────────────────────────────────────────────────────────────┤
│  L4  Домены:                                                │
│      physics (particles → rigid2d → rigid3d → soft/fluid)    │
│      scene  (ESC-сцена, синхронизация, сериализация)         │
│      assets (этап 7)                                        │
├──────────────────────────────────────────────────────────────┤
│  L3  Рендер-API: IRenderer, IDebugDraw, Camera               │
│      реализации бэкендов: sfml → gl → vk                     │
├──────────────────────────────────────────────────────────────┤
│  L2  Сервисы: ecs (entt-like), jobs (enkiTS-like),           │
│      events (event bus)                                     │
├──────────────────────────────────────────────────────────────┤
│  L1  Платформа: окно, ввод, время (IPlatform)                │
├──────────────────────────────────────────────────────────────┤
│  L0  Фундамент: math (GLM-like), core (log, handle, memory)  │
└──────────────────────────────────────────────────────────────┘
```

Ключевое правило треков: **физика зависит только от L0 и от `render_api` (заголовочный интерфейс L3), но не от реализаций рендера.** Рендер не зависит от физики вообще. Точка встречи — сцена (L4) и приложения (L6).

---

## 3. Структура репозитория

```
temka/
├── CMakeLists.txt               # корневой: опции, таргеты warnings/sanitizers
├── cmake/                       # хелперы (Warnings.cmake, Sanitizers.cmake)
├── engine/
│   ├── core/         → tml_core            # лог, хэндлы, пулы, профайлер, assert
│   ├── math/         → tml_math  (header-only)
│   ├── platform/     → tml_platform_api    # IPlatform, WindowDesc, InputState
│   │                 → tml_platform_sfml   # реализация (обёртка SFML)
│   ├── events/       → tml_events          # типизированный event bus
│   ├── jobs/         → tml_jobs            # пул потоков + parallelFor
│   ├── ecs/          → tml_ecs   (header-only, entt-like)
│   ├── render/
│   │   ├── api/      → tml_render_api (header-only: IRenderer, IDebugDraw, Camera, Color)
│   │   ├── sfml/     → tml_render_sfml     # этапы 1–3 (2D debug-стенд)
│   │   ├── gl/       → tml_render_gl       # этап 4 (OpenGL 3.3+)
│   │   └── vk/       → tml_render_vk       # этап 4 (поздняя часть)
│   ├── physics/
│   │   ├── common/   → tml_physics_common  # интеграторы, IWorld2D-интерфейс, типы
│   │   ├── particles/→ tml_physics_particles # этап 2
│   │   ├── rigid2d/  → tml_physics_rigid2d   # этап 3
│   │   ├── rigid3d/  → tml_physics_rigid3d   # этап 5
│   │   └── soft/     → tml_physics_soft      # этап 6 (XPBD, SPH, fracture)
│   ├── scene/        → tml_scene            # ECS-сцена, sync-системы, сериализация
│   ├── assets/       → tml_assets           # этап 7 (эскиз ниже)
│   └── engine/       → tml_engine           # фасад: Engine, Layer, game loop
├── apps/
│   ├── playground/   → temka_playground     # визуальный отладчик физики (этапы 1–3)
│   └── editor/       → temka_editor         # этап 8
├── tests/            → temka_tests          # unit-тесты всех модулей
├── extern/                                 # SFML, Dear ImGui — единственные внешние зависимости
├── organization/                          # планы и этот документ
└── materials/
```

Каждый модуль: `include/tml/<module>/*.h` (публичный API) + `src/*.cpp` (реализация). Пространство имён движка — `tml`, подпространства: `tml::math`, `tml::ecs`, `tml::phys`, `tml::rnd` и т.д.

**Внешние зависимости (допущение, см. §12):** только SFML (окно + GL-контекст) и Dear ImGui (UI редактора). Всё остальное — свой код по образцу готовых библиотек.

---

## 4. Каталог модулей

| Модуль | Слой | Ответственность | Зависит от |
|---|---|---|---|
| `tml_core` | L0 | лог, assert, хэндлы, pool/arena, профайлер | — |
| `tml_math` | L0 | vec2/3/4, mat3/4, quat, transform, AABB, ray, plane | — |
| `tml_platform_api` | L1 | интерфейсы окна/ввода/времени | core |
| `tml_platform_sfml` | L1 | реализация поверх SFML | platform_api, extern SFML |
| `tml_events` | L2 | типизированный event bus с отложенной доставкой | core |
| `tml_jobs` | L2 | пул потоков, parallelFor, счётчики зависимостей | core |
| `tml_ecs` | L2 | registry, компонентные пулы, view | core |
| `tml_render_api` | L3 | интерфейсы IRenderer/IDebugDraw, камера, цвет | math |
| `tml_render_sfml` | L3 | 2D-бэкенд отладочной графики | render_api, extern SFML |
| `tml_physics_common` | L4 | интеграторы, интерфейс мира, общие типы | math, render_api |
| `tml_physics_particles` | L4 | частицы, силы, constraints | physics_common |
| `tml_physics_rigid2d` | L4 | 2D твёрдые тела, broad/narrowphase, solver | physics_common |
| `tml_scene` | L4 | ECS-сцена, компоненты, sync физика↔ECS, сериализация | ecs, physics_common, render_api |
| `tml_engine` | L5 | фасад, жизненный цикл, game loop, Layer stack | platform, events, jobs, render_api |
| `tml_assets` | L4/L7 | менеджер ресурсов, загрузчики (этап 7) | core, jobs, ecs |
| `temka_playground` | L6 | приложение-песочница | engine, physics_*, render_sfml |
| `temka_editor` | L6 | редактор (этап 8) | engine, scene, render_*, extern ImGui |

Правило сборки: таргет ссылается только на таргеты своей строки «Зависит от». Нарушение = ошибка ревью.

---

## 5. API модулей

Все сигнатуры ниже — канонические имена и формы; детали аргументов можно уточнять, форму — нельзя (она и есть договор).

### 5.1 `tml_core` — фундамент

```cpp
namespace tml {

// ---- Логирование (по образцу spdlog: уровни + каналы-теги) ----
enum class LogLevel : uint8_t { Trace, Debug, Info, Warn, Error, Fatal };

namespace log {
    void setLevel(LogLevel level);
    void setLevel(const char* channel, LogLevel level);
    void message(LogLevel level, const char* channel, const char* fmt, ...)
        __attribute__((format(printf, 3, 4)));
}

#define TM_TRACE(ch, ...) ::tml::log::message(::tml::LogLevel::Trace, ch, __VA_ARGS__)
#define TM_DEBUG(ch, ...) ::tml::log::message(::tml::LogLevel::Debug, ch, __VA_ARGS__)
#define TM_INFO(ch,  ...) ::tml::log::message(::tml::LogLevel::Info,  ch, __VA_ARGS__)
#define TM_WARN(ch,  ...) ::tml::log::message(::tml::LogLevel::Warn,  ch, __VA_ARGS__)
#define TM_ERROR(ch, ...) ::tml::log::message(::tml::LogLevel::Error, ch, __VA_ARGS__)

// ---- Assert (в release превращается в лог + abort по политике) ----
#define TM_ASSERT(cond, msg) ...
#define TM_ASSERT_MSG(cond, fmt, ...) ...

// ---- Поколенческий хэндл: 8 байт, стабильный при реаллокациях ----
struct Handle32 {
    uint32_t index  = 0xFFFFFFFFu;   // 0xFFFFFFFF — невалидный
    uint32_t gen    = 0;
    bool isValid() const { return index != 0xFFFFFFFFu; }
    bool operator==(const Handle32&) const = default;
};

template <typename Tag> struct Handle : Handle32 {};   // Handle<BodyTag>, Handle<MeshTag>...

// ---- Пул с поколениями (базовый строительный блок всех модулей) ----
template <typename T, typename Tag = T>
class Pool {
public:
    Handle<Tag> create(T&& value);
    T*          tryGet(Handle<Tag> h);        // nullptr, если поколение не совпало
    const T*    tryGet(Handle<Tag> h) const;
    void        destroy(Handle<Tag> h);
    uint32_t    size() const;
    template <typename F> void forEach(F&& fn);   // плотная итерация по живым
};

// ---- Профайлер: трассировочные события, экспорт в Chrome Trace (about:tracing) ----
namespace prof {
    void beginFrame();
    void scope(const char* name);   // с thread-local стека областей
    void event(const char* name, float ms);
    void dump(const char* path);    // JSON в формате Chrome Trace
}
struct ProfileScope {               // RAII
    explicit ProfileScope(const char* name);
    ~ProfileScope();
};

} // namespace tml
```

Свои allocator'ы (arena/pool) добавляются сюда же по мере надобности, API — `makeUnique<T>(Arena&)`.

### 5.2 `tml_math` — математика (по образцу GLM)

```cpp
namespace tml::math {

struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };
struct Mat3 { float m[9]; };            // column-major, как в OpenGL
struct Mat4 { float m[16]; };
struct Quat { float x, y, z, w; };

struct Transform {                       // минимальное представление позы
    Vec3 position{0,0,0};
    Quat rotation{};                     // identity
    Vec3 scale{1,1,1};
    Mat4 toMat4() const;
};

struct AABB { Vec3 min, max; void expand(const Vec3& p); bool intersects(const AABB&) const; };
struct Plane { Vec3 normal; float d; };
struct Ray  { Vec3 origin, dir; };       // dir нормализован

// Полный набор операций: +,-,*,dot,cross,length,normalize, lerp,
// mat mul, inverse, transpose, lookAt, ortho, perspective,
// quat fromAxisAngle/toMat4/slerp, transformPoint/Vector.

} // namespace tml::math
```

Замечание по 2D-этапам: 2D-код использует `Vec2` и `Mat4` от `ortho`-проекции. 3D-переход не требует переписывания — модули физики 2D и 3D раздельные (см. 5.7), математика общая.

### 5.3 `tml_platform` + `tml_events` — окно, ввод, события

```cpp
namespace tml {

struct WindowDesc {
    const char* title = "temka";
    uint32_t width = 1280, height = 720;
    bool resizable = true;
    bool vsync = true;
};

struct InputState {                       // снимок на кадр
    bool keyDown[static_cast<size_t>(Key::Count)];
    bool keyPressed[...];                 // «нажато в этом кадре»
    bool mouseDown[static_cast<size_t>(Mouse::Count)];
    Vec2i mousePos, mouseDelta;
    Vec2i wheelDelta;
};

class IPlatform {                         // единственный шов к ОС/библиотеке
public:
    virtual ~IPlatform() = default;
    virtual bool init(const WindowDesc& desc) = 0;
    virtual bool pollEvents() = 0;        // false → окно закрыто
    virtual const InputState& input() const = 0;
    virtual void* nativeWindowHandle() = 0;  // для создания GL-контекста рендером
    virtual double timeSeconds() const = 0;
    virtual void requestClose() = 0;
};

IPlatform& platform();                    // установлена при старте Engine

} // namespace tml
```

```cpp
namespace tml {

// Типизированная шина с отложенной доставкой: publish потокобезопасен,
// dispatch() вызывается на главном потоке в начале кадра.
class EventBus {
public:
    template <typename E> void publish(E&& event);          // в очередь
    template <typename E> void subscribe(std::function<void(const E&)>);
    void dispatch();                                        // доставить накопленное
    void clear();
};

// Базовые события платформы (расширяются приложением своими структурами)
struct WindowResized { uint32_t width, height; };
struct WindowClosed {};
struct KeyPressed { Key key; bool repeat; };
struct MouseButtonPressed { Mouse button; Vec2i pos; };

} // namespace tml
```

### 5.4 `tml_jobs` — job system (по образцу enkiTS)

```cpp
namespace tml {

class JobSystem {
public:
    explicit JobSystem(uint32_t threads = 0);  // 0 → hw_concurrency − 1
    ~JobSystem();

    struct Counter { std::atomic<uint32_t> value{0}; };

    // Разбивает [0, count) на батчи и исполняет fn(begin, end) на пуле.
    // Синхронный вызов с главного потока допустим (main помогает пулу).
    template <typename F>
    void parallelFor(uint32_t count, uint32_t minBatch, F&& fn,
                     Counter* optionalDependency = nullptr);

    // Одиночная задача + зависимость (этап 6: параллельные острова физики).
    template <typename F>
    void run(F&& fn, Counter* optionalDependency = nullptr);

    void waitFor(Counter& c);                  // активное ожидание с work-stealing

    uint32_t threadCount() const;
    uint32_t threadIndex() const;              // внутри задачи
};

} // namespace tml
```

Ограничение на первом этапе: физика однопоточна, `parallelFor` обкатывается на particles (обновление позиций) и в asset loader. Это не переделка API позже — только добавление вызовов.

### 5.5 `tml_ecs` — ECS (по образцу entt)

```cpp
namespace tml::ecs {

using Entity = uint64_t;   // младшие 32 бита — индекс, старшие — поколение

class Registry {
public:
    Entity create();
    void destroy(Entity e);
    bool   valid(Entity e) const;

    template <typename C, typename... Args>
    C& emplace(Entity e, Args&&... args);
    template <typename C> C&       getOrEmplace(Entity e);
    template <typename C> C*       tryGet(Entity e);
    template <typename C> bool     has(Entity e) const;
    template <typename C> void     remove(Entity e);
    template <typename C> void     clear();

    // Итерация по сущностям, владеющим всеми перечисленными компонентами.
    template <typename... C, typename F>
    void each(F&& fn);                        // fn(Entity, C&...)

    // Сигналы (для sync-систем сцены): вызываются при изменении состава.
    template <typename C> void onConstruct(std::function<void(Entity, C&)>);
    template <typename C> void onDestroy(std::function<void(Entity, C&)>);
};

} // namespace tml::ecs
```

Хранение: плотные массивы на компонент (SoA внутри пула), индексация через sparse-таблицу сущностей. Хэндлы физики (`phys::BodyId`) живут **внутри** компонентов, а не наоборот.

### 5.6 `tml_render` — рендер-API и отладочная графика

Это контракт, ради которого существует документ: он переживает смену SFML → OpenGL → Vulkan.

```cpp
namespace tml::rnd {

struct Color { float r, g, b, a; };

struct Camera {                            // унифицированная для 2D и 3D
    math::Mat4 viewProj;
    math::AABB frustumAABB;                // для отсечения debug-графики
    // Строится фабриками:
    static Camera ortho2D(math::Vec2 center, float zoom, float aspectWtoH);
    static Camera perspective(math::Vec3 pos, math::Quat look,
                              float fovYRad, float aspectWtoH,
                              float nearZ, float farZ);
};

// ---- Контракт отладочной отрисовки. Физика знает ТОЛЬКО его. ----
class IDebugDraw {
public:
    virtual ~IDebugDraw() = default;

    virtual void begin(const Camera& camera) = 0;
    virtual void end() = 0;                 // flush батчей

    virtual void point(math::Vec3 p, Color c) = 0;
    virtual void line(math::Vec3 a, math::Vec3 b, Color c) = 0;
    virtual void polyline(const math::Vec3* pts, uint32_t n, Color c, bool closed) = 0;

    virtual void circle(math::Vec3 center, float radius, Color c) = 0;   // в плоскости XY
    virtual void aabb(const math::AABB& box, Color c) = 0;
    virtual void sphere(math::Vec3 center, float radius, Color c) = 0;
    virtual void triad(math::Vec3 origin, float size) = 0;               // RGB = XYZ
    virtual void arrow(math::Vec3 from, math::Vec3 to, Color c) = 0;
    virtual void text(math::Vec3 pos, const char* utf8, Color c) = 0;    // этап 1.5

    virtual void pushLayer();               // слои: overlay поверх симуляции
    virtual void popLayer();
};

// ---- Общий интерфейс рендера ----
class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual bool init(IPlatform& platform, const WindowDesc& desc) = 0;
    virtual void shutdown() = 0;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;            // present

    virtual IDebugDraw& debug() = 0;

    // Расширение этапа 4 (НЕ добавляем сейчас, чтобы не раздувать контракт):
    //   virtual DrawList& draws() = 0;      // меши, материалы, тени
    //   virtual IRenderDevice& device() = 0;// буферы, шейдеры, текстуры

    virtual const char* backendName() const = 0;   // "sfml" | "gl" | "vulkan"
};

// Фабрика в библиотеке бэкенда: tml_render_sfml и т.д.
UniquePtr<IRenderer> createRenderer();

} // namespace tml::rnd
```

Правила контракта `IDebugDraw`:
- 2D-сцена задаёт `z = 0`; никакой отдельной `IDebugDraw2D` не существует — 2D есть частный случай 3D. Это главное решение, спасающее от переписывания на этапе 4.
- Реализация батчит вызовы в вершинные буферы; между `begin/end` — неограниченное число вызовов.
- `pushLayer/popLayer` рендерятся в порядке стекa поверх базового.

### 5.7 `tml_physics` — физика

Структура трека: `physics_common` (интерфейс + интеграторы + общие типы) и отдельные миры. **Каждый этап плана = новый подмодуль, существующие не переписываются.**

```cpp
namespace tml::phys {

using BodyId = Handle<struct BodyTag>;

// ---- physics_common: интеграторы (сравниваем на этапе 2) ----
enum class Integrator { ExplicitEuler, SemiImplicitEuler, Verlet, RK4 };

// ---- Общий интерфейс 2D-мира: его форму копирует rigid3d::IWorld ----
class IWorld2D {
public:
    virtual ~IWorld2D() = default;

    virtual BodyId createBody(const BodyDesc2D& desc) = 0;
    virtual void   destroyBody(BodyId id) = 0;

    virtual void   setGravity(math::Vec2 g) = 0;

    virtual void   step(float fixedDt) = 0;              // только фиксированный шаг!

    // Состояние для рендера/сцены (позиция + интерполяция между шагами)
    virtual math::Vec2 position(BodyId id) const = 0;
    virtual math::Vec2 interpolatedPosition(BodyId id, float alpha) const = 0;
    virtual void       setTransform(BodyId id, math::Vec2 pos, float angle) = 0;

    virtual void   applyForce(BodyId id, math::Vec2 force) = 0;
    virtual void   applyImpulse(BodyId id, math::Vec2 impulse) = 0;

    virtual bool   raycast(const math::Vec2& from, const math::Vec2& dir,
                           float maxDist, RaycastHit2D& out) const = 0;

    virtual void   debugDraw(rnd::IDebugDraw& dd) const = 0;   // <-- единственная связь с рендером

    // События контактов (этап 3): слушатель — необязательный weak-указатель
    struct IContactListener {
        virtual void onContact(const ContactEvent2D&) {}
        virtual ~IContactListener() = default;
    };
    virtual void setContactListener(IContactListener* listener) = 0;
};

struct BodyDesc2D {                  // расширяемая POD-структура: новые поля не ломают API
    math::Vec2 position{0, 0};
    float      angle = 0.f;
    math::Vec2 velocity{0, 0};
    float      angularVelocity = 0.f;
    float      mass = 1.f;           // 0 = static
    float      restitution = 0.5f;
    float      friction = 0.3f;
    // этап 2+:  ShapeRef shape;   этап 3+:  ShapeType type; ...
};

} // namespace tml::phys
```

Подмодули-реализации:

| Подмодуль | Класс | Что добавляет сверх `IWorld2D` |
|---|---|---|
| `physics_particles` | `ParticleWorld : IWorld2D` | поля сил (гравитация/drag/пружины), distance constraints, контакты с границами |
| `physics_rigid2d` | `RigidWorld2D : IWorld2D` | момент инерции, broadphase (grid → AABB-tree), SAT, sequential impulse solver, sleeping, острова, джойнты |
| `physics_rigid3d` | `RigidWorld3D : IWorld3D` | GJK/EPA, 3D-джойнты, CCD; `IWorld3D` — зеркальная форма `IWorld2D` на Vec3/Quat |
| `physics_soft` | `SoftWorld` (этап 6) | XPBD-тела, ткань, SPH; живёт параллельно с rigid, объединяются на уровне сцены |

Правило: сцена и приложения общаются с миром **только** через интерфейс + собственные расширяющие методы конкретного класса (например, `ParticleWorld::addSpring(a, b, k)`), когда фича нужна сцене. «Божественного» интерфейса не делаем.

### 5.8 `tml_scene` — сцена и мост физика↔ECS

```cpp
namespace tml::scene {

// Компоненты — тонкие обёртки над данными физики/рендера
struct TransformComponent { math::Transform local; };   // иерархия через ParentComponent
struct RigidBody2D {
    phys::BodyId      body;         // невалиден, пока не создан sync-системой
    phys::BodyDesc2D  desc;         // редактируется в ImGui/редакторе
};
struct DebugDrawComponent { rnd::Color color; bool drawAABB; };

class Scene {
public:
    explicit Scene(phys::IWorld2D& world);     // сцена НЕ владеет миром

    ecs::Registry& registry();

    // Системы (вызываются из Scene::fixedUpdate / Scene::render)
    void fixedUpdate(float fixedDt);   // syncToPhysics → world.step → syncFromPhysics
    void render(rnd::IRenderer& renderer, float alpha);

    // Сериализация (этап 7): сохранить/загрузить состав сущностей и descs
    bool save(const char* path) const;
    bool load(const char* path);

private:
    // Sync-система: onConstruct<RigidBody2D> → world.createBody(desc);
    //               onDestroy<RigidBody2D>   → world.destroyBody(body);
    //               после step: body.position → TransformComponent.
};

} // namespace tml::scene
```

Схема владения: **`Scene` ссылается на физический мир по указателю/ссылке, но не создаёт его.** Мир создаёт приложение (или Engine по конфигу) и передаёт в сцену. Это позволяет подменять `ParticleWorld` ↔ `RigidWorld2D` без правки сцены.

### 5.9 `tml_assets` — ресурсы (этап 7, эскиз)

```cpp
namespace tml::assets {

using AssetId = Handle<struct AssetTag>;

class AssetManager {
public:
    AssetId load(const char* path);               // по расширению выбирает загрузчик
    template <typename T> const T* get(AssetId);  // + refcount
    void update();                                // достраивает загрузки из очереди jobs
};

// Загрузчики регистрируются: gltf-меш, текстура, шейдер, .tmesh (этап 8)
using LoaderFn = bool(*)(const char* path, void* outStorage);

} // namespace tml::assets
```

API зафиксирован минимально; детали (типы ресурсов, hot-reload, асинхронность) прорабатываются на этапе 7. Горячая точка расширения: новые типы ресурсов = новый загрузчик, без правки менеджера.

### 5.10 `tml_engine` — фасад и game loop

```cpp
namespace tml {

struct EngineConfig {
    WindowDesc window;
    float      fixedDt = 1.f / 60.f;      // шаг физики
    uint32_t   maxStepsPerFrame = 5;     // защита от «спирали смерти»
    LogLevel   logLevel = LogLevel::Info;
};

// Слой приложной логики (по образцу Hazel): редактор и playground — это наборы слоёв
class Layer {
public:
    virtual ~Layer() = default;
    virtual void onAttach(Engine&) {}
    virtual void onDetach() {}
    virtual void onEvent(const EventBus&) {}       // после dispatch()
    virtual void onFixedUpdate(float fixedDt) {}   // логика + физика
    virtual void onUpdate(float dt, float alpha) {}
    virtual void onRender(rnd::IRenderer&) {}
};

class Engine {
public:
    int run(Layer* appLayers, size_t layerCount);  // блок до закрытия окна

    // Доступ подсистем для слоёв
    rnd::IRenderer& renderer();
    JobSystem&      jobs();
    EventBus&       events();
    const InputState& input() const;

    // Регистрация «движковых» модулей (расширение без правки Engine):
    // физический мир, аудио и т.п. реализуют IModule и получают колбэки цикла
    struct IModule {
        virtual ~IModule() = default;
        virtual bool init(Engine&) { return true; }
        virtual void fixedUpdate(float) {}
        virtual void update(float) {}
        virtual void render(rnd::IRenderer&) {}
        virtual void shutdown() {}
    };
    void addModule(UniquePtr<IModule> module);
};

} // namespace tml
```

**Канонический цикл кадра** ( Glenn Fiedler, «Fix Your Timestep!»):

```
while (running) {
    // 1. Ввод и события
    platform.pollEvents();               →   InputState, publish(события)
    events.dispatch();                   →   слои получают onEvent

    // 2. Физика фиксированным шагом
    accumulator += min(frameDelta, maxFrameDelta);
    while (accumulator >= fixedDt && steps < maxStepsPerFrame) {
        for (layer : layers) layer.onFixedUpdate(fixedDt);   // внутри: world.step(fixedDt)
        accumulator -= fixedDt; ++steps;
    }
    alpha = accumulator / fixedDt;

    // 3. Рендер
    renderer.beginFrame();
    for (layer : layers) layer.onRender(renderer);   // внутри: world.debugDraw(dd), интерполяция alpha
    renderer.endFrame();
}
```

Интерполяция: рендер использует `interpolatedPosition(id, alpha)`, чтобы визуально гладко видеть 60 Гц физику на 144 Гц мониторе.

---

## 6. Межмодульные договоры («трактаты треков»)

Подписываются до начала параллельной работы. Изменение трактата = отдельный коммит-предложение + обсуждение.

1. **Трактат рендера.** Физика и сцена включают только `render/IDebugDraw.h` (+ `Camera`, `Color`) из `tml_render_api`. Им запрещено включать заголовки бэкендов. Бэкенды не включают заголовки физики.
2. **Трактат физики.** Никто вне `tml_physics_*` не трогает внутренние структуры мира. Доступ — только `IWorld2D`/`IWorld3D` + `Handle`. Тело живёт в мире; в ECS — только хэндл и desc.
3. **Трактат данных.** Через границы модулей передаются: значения, хэндлы, POD-структуры-desc. Не передаются: сырые указатели на внутренние контейнеры, `std::function`-колбэки вглубь чужого модуля (кроме явных listener-интерфейсов).
4. **Трактат времени.** Все подмодули физики получают время только как аргумент `step(fixedDt)`. Свое время/`std::chrono::now()` внутри симуляции запрещено (детерминизм).
5. **Трактат потоков.** Главный поток: события, sync-системы, submit рендера. Пул jobs: внутренности физики (после этапа 5), asset IO. Мутации ECS — только на главном потоке.
6. **Обратная совместимость API.** Новые поля добавляются в desc-структуры с дефолтами. Методы из интерфейсов не удаляются и не меняют сигнатуру — депрекация через `[[deprecated]]` + период сосуществования.

---

## 7. Потоковая модель

| Поток | Работа |
|---|---|
| main | pollEvents, events.dispatch, sync-системы ECS, `world.step`, submit рендера, ImGui |
| worker × (cores−1) | `parallelFor` внутри step (этап 5+: острова, solver), загрузка ассетов (этап 7) |

До этапа 5 физика выполняется на main, но **кодируется сразу так**, чтобы внутренние данные мира не имели скрытых глобалов: `parallelFor` затем применяется без рефакторинга API.

---

## 8. Точки расширения (как расти не переписывая)

| Добавляем | Что делаем | Что НЕ трогаем |
|---|---|---|
| OpenGL/Vulkan-бэкенд | новый таргет `tml_render_gl`, реализует `IRenderer`/`IDebugDraw` | физику, сцену, playground |
| 3D-физика | `physics_rigid3d::RigidWorld3D : IWorld3D` | `IWorld2D`, частицы, рендер |
| Soft body / SPH | `physics_soft::SoftWorld` + компонент `SoftBodyComponent` | solver rigid-мир |
| Скриптинг (Lua/sol2 или свой) | новый слой `tml_script` над `scene` + reflection-таблицы компонентов | ядро |
| Редактор | приложение `temka_editor`: слои + ImGui + command pattern (undo) | движковые модули (только их API) |
| Аудио | `tml_audio : Engine::IModule` | всё |
| Подбор параметров по видео (уникальная фича) | модуль `calibration`: читает трек точек из видео, гоняет детерминированные реплеи сцены, оптимизирует параметры `BodyDesc` | физика уже детерминирована — этим и окупается трактат времени |
| Новый формат ассетов | загрузчик в `AssetManager` | менеджер |

---

## 9. Сборка (CMake)

- Multi-config-дружелюбно; пресеты: `Debug` (asserts, `-O0 -g`), `RelWithDebInfo`, `Sanitize` (ASan/UBSan), `Analyze` (clang-tidy по `cmake/`).
- `-Wall -Wextra -Werror` для своих таргетов; внешние библиотеки собираются с `SYSTEM` include.
- `tml_math`, `tml_ecs`, `tml_render_api`, `tml_platform_api` — `INTERFACE` (header-only): их «зависимость» стоит копейки, интерфейсы стабильны.
- Зависимости между таргетами объявляются через `target_link_libraries(... PUBLIC ...)`, никогда через глобальные include-пути.
- Тесты: `temka_tests` (ctest), один исполняемый файл, регистрация по секциям модулей.

## 10. Тестирование

- **Unit:** math (все операции с эталонами), ecs (создание/удаление/поколения), jobs (parallelFor корректность, гонки под TSan), core (pool, handle).
- **Физика — golden replay:** сцена фиксируется в файл; шаги прогона сравниваются с эталонной траекторией побайтово (float). Ловит регрессии детерминизма и стабильности интеграторов.
- **Визуальный smoke:** playground запускает демо-сцену; критерий — не упало и число объектов/контактов в статистике совпало.
- Свой мини-фреймворк `tml_test` (doctest-подобный: `TM_TEST("имя") { TM_CHECK(...); }`, авторегистрация) — он тривиален и даёт ноль внешних зависимостей.

## 11. Соответствие этапам projectPlan.md

| Этап плана | Какие модули создаются/растут |
|---|---|
| 0 — инфраструктура | `core`, `math`, `platform`, `events`, `jobs`, `ecs`, CMake, тесты |
| 1 — тестовый стенд | `render_api`, `render_sfml`, `engine` (loop), `playground` |
| 2 — частицы | `physics_common`, `physics_particles`, интеграторы |
| 3 — 2D rigid | `physics_rigid2d`, контактные события, статистика в playground |
| 4 — свой рендер | `render_gl` → `render_vk`, DrawList (расширение `render_api`) |
| 5 — 3D физика | `physics_rigid3d`, jobs внутри solver, острова |
| 6 — soft/fluids/fracture | `physics_soft` (XPBD, SPH, Voronoi) |
| 7 — интеграция | `assets`, сериализация `scene`, `tml_script` |
| 8 — редактор | `temka_editor`, `.tmesh`, undo/redo |
| 9 — полировка | профайлер, многопоточность физики, IPC |

## 12. Принятые допущения и открытые вопросы

**Допущения (снять/подтвердить):**
1. SFML и Dear ImGui — единственные внешние зависимости (окно и UI — инфраструктура, а не учебные модули). Всё остальное пишем сами.
2. Логирование — printf-style форматные строки.
3. Версионирование/совместимость форматов сцен появится на этапе 7; до этого формат `.scene` считается нестабильным.

**Открытые вопросы:**
1. Компиляторы-минимумы (GCC? Clang? оба + MinGW для Windows на этапе 9).
2. Точка входа в 2D-физике: начать с `Vec2`-мира частиц (как в плане) — подтверждено планом, но зафиксировать, что `IWorld3D` появится зеркально на этапе 5.
3. Профайлер: сначала свой Chrome-Trace-экспорт (§5.1), Tracy как опция позже — обсуждается на этапе 0.
