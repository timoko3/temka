# Полный план разработки движка (temka.exe)

> Цель: 3D-движок с физикой (твёрдые тела + мягкие тела + жидкости + разрушаемость) и встроенным 3D-редактором моделей.  
> Подход: смешанный — учебные модули пишем сами (физика, рендер), инфраструктуру берём готовую (enkiTS, GLM, ImGui, glTF).  
> Язык: C++. Графика: SFML → OpenGL → Vulkan. UI редактора: immediate mode (Dear ImGui).

План выстроен как каскад: каждый этап либо расширяет предыдущий, либо становится фундаментом для следующего. Этапы можно проходить параллельно несколькими «треками» (рендер / физика / инструменты), договариваясь об интерфейсах.

---

## Этап 0 — Инфраструктура и фундамент

Базовая работа, на которой держится всё остальное. Делать **прежде всего**, иначе потом придётся переписывать подсистемы.

### Подзадачи
- [ ] Сборка: CMake, multi-config, sanitize-таргеты (ASan/UBSan), статический анализатор (clang-tidy).
- [ ] Окно и ввод: обёртка над SFML (`Window`, `Input`, `Mouse/Keyboard`), абстракция `IPlatform` под будущий GLFW/Vulkan.
- [ ] Логирование с уровнями и каналами (`spdlog` или тонкая обёртка).
- [ ] Математическая библиотека: `vec2/3/4`, `mat3/4`, `quat`, `transform`, AABB, plane, ray. Выбор: `GLM` (быстро начать) или свой слой поверх GLM для понимания.
- [ ] Контейнеры и аллокаторы: arena/pool allocator, `Handle<T>` вместо сырых указателей, SoA-layout.
- [ ] ECS (Entity-Component-System): готовый `entt` (рекомендую) или минимальный свой — для понимания.
- [ ] Job system: `enkiTS` (production) или мини-пул потоков с work-stealing (обучение).
- [ ] Система событий (event bus) + signal/slot.
- [ ] Профайлер: `Tracy` — поставить сразу и привыкнуть профилировать.
- [ ] Тесты: `Catch2`/`doctest` — unit-тесты на математику и физику.
- [ ] Конвенции кодстайла: `.clang-format`, `.clang-tidy`.

### Ресурсы
- Книга: **Jason Gregory — Game Engine Architecture (3rd ed.)** — структура движка, подсистемы, паттерны.
- Книга: **Mike McShaffry — Game Coding Complete** — практические рецепты.
- Книга: **Eric Lengyel — Foundations of Game Engine Development, Vol 1: Mathematics**.
- ECS: https://github.com/skypjack/entt (wiki отличная).
- enkiTS: https://github.com/dougbinks/enkiTS.
- Tracy: https://github.com/wolfplasma/tracy + доклады на YouTube.
- C++ Best Practices: https://github.com/lefticus/cppbestpractices.

---

## Этап 1 — Графический тестовый стенд (SFML)

Цель: визуальный отладчик физики. Сразу договариваемся об интерфейсе `IRenderer`/`IDebugDraw` — чтобы потом подменить SFML на OpenGL/Vulkan без переписывания физики.

### Подзадачи
- [ ] Интерфейс `IDebugDraw`: линии, точки, AABB, круги, текст, стрелки, триады.
- [ ] Камера 2D (pan/zoom), ортогональная проекция.
- [ ] Слои рендера (debug overlay поверх симуляции).
- [ ] ImGui-оверлей для редактирования параметров физики в реальном времени.
- [ ] FPS-счётчик, статистика по сцене (объекты, контакты, шаги).

### Ресурсы
- SFML tutorials: https://www.sfml-dev.org/tutorials/.
- Dear ImGui: https://github.com/ocornut/imgui (binding `imgui-sfml`).
- Hazel engine (The Cherno) — YouTube-серия, отличный референс по архитектуре: https://www.youtube.com/@TheCherno.

---

## Этап 2 — Движок частиц (первый этап физики)

Соответствует `firstVersionPlan.md`. Учим основы численного интегрирования и сил. Параллельно используем тестовый стенд из Этапа 1.

### Подзадачи
- [ ] Представление частицы (position, velocity, force accumulator, invMass).
- [ ] Интеграторы: явный/полу-неявный Euler, Verlet, RK4 — сравнить стабильность.
- [ ] Силы: гравитация, drag, пружины, притяжение/отталкивание.
- [ ] Контакты частиц с границами (отскок с restitution).
- [ ] Constraint-система для связывания частиц (distance constraint) — база для soft body позже.
- [ ] Fixed timestep с аккумулятором (канонический game-loop Фиксированного шага).

### Ресурсы
- **Ian Millington — Game Physics Engine Development** — базовая книга, именно с неё начать.
- Glenn Fiedler «Fix Your Timestep!» — https://gafferongames.com/post/fix_your_timestep/.
- Glenn Fiedler — серия статей «Game Physics» на том же сайте (gafferongames.com).
- Matthias Müller (NVIDIA) — Ten Minute Physics: https://matthias-research.github.io/pages/ten-minute-physics/.

---

## Этап 3 — 2D твердотельная динамика

Расширяем частицы до тел с моментом инерции. Здесь отшлифовываем алгоритмы, которые в 3D будут только усложнены.

### Подзадачи
- [ ] Rigid body: position, orientation, linear/angular velocity, invMass, invInertia.
- [ ] Broadphase: uniform grid, sweep-and-prune, dynamic AABB tree — сравнить.
- [ ] Narrowphase: circle/circle, circle/box, box/box (SAT).
- [ ] Разрешение контактов: impulse-based, sequential impulse (Erin Catto).
- [ ] Friction (Coulomb) и restitution.
- [ ] Constraints: point-to-point, weld, hinge, distance, revolute.
- [ ] Solver: Sequential Impulse с warm-starting и bias.
- [ ] Sleeping и острова (island detection via union-find).
- [ ] CCD (continuous collision detection) для быстрых тел — базово.

### Ресурсы
- **Box2D source code** (https://github.com/erincatto/box2d) — эталонная реализация, читать исходники.
- **Erin Catto — GDC slides** «Solving Rigid Body Contacts» и др.: https://box2d.org/publications/.
- **Christer Ericson — Real-Time Collision Detection** — bible по обнаружению столкновений.
- Dirk Gregorius GDC talks «Physics for Programmers» — на YouTube/GDC Vault.

---

## Этап 4 — Переход на собственный 3D-рендер (OpenGL → Vulkan)

Заменяем SFML на собственный рендер. Это та самая «вилка» из `firstVersionPlan.md`: отладочная графика вырастает в полноценный рендер-движок.

### Подзадачи
- [ ] Бэкенд `IRenderer` с реализацией на OpenGL 3.3+.
- [ ] Буферы, VAO/VBO/IBO, шейдеры (GLSL), uniform-буферы.
- [ ] Трансформации в 3D, кватернионы, иерархия трансформаций (scene graph).
- [ ] Загрузка мешей: формат glTF (библиотека `cgltf` или `tinygltf`).
- [ ] Текстуры и сэмплеры (stb_image).
- [ ] Камеры: perspective, orbit, FPS, компонент Camera.
- [ ] Forward rendering: направленный свет, точечные/прожекторные источники, ambient.
- [ ] Material/Shader system: универсальный материал с параметрами.
- [ ] Скайбокс, тени (shadow map, directional light).
- [ ] Переход на Vulkan: `vulkan-tutorial.com` → `vkguide.dev`, render graph, descriptor sets.
- [ ] Абстракция backend-agnostic (render API abstraction layer).

### Ресурсы
- **LearnOpenGL** (Joey de Vries) — https://learnopengl.com/ (лучший вход в современный OpenGL).
- **Vulkan Tutorial** — https://vulkan-tutorial.com/.
- **Vulkan Guide** — https://vkguide.dev/ (более современный подход с render graph).
- **Scratchapixel** — https://www.scratchapixel.com/ (рендеринг, ray tracing, основы).
- **The Book of Shaders** — https://thebookofshaders.com/.
- Catlike Coding (Jasper Flick) — https://catlikecoding.com/ (серии по рендеру, хоть и на Unity — концепции переносимы).
- **Real-Time Rendering** (Akenine-Möller, Haines, Hoffman) — bible по рендеру.
- Sascha Willems Vulkan samples: https://github.com/SaschaWillems/Vulkan.

---

## Этап 5 — 3D физика твердых тел

Переносим наработки Этапа 3 в 3D. Главные новые элементы — GJK/EPA и 3D-ориентации.

### Подзадачи
- [ ] 3D-формы: sphere, box, capsule, convex hull, triangle mesh, compound.
- [ ] Broadphase: dynamic AABB tree (BVH) + sweep-and-prune по осям.
- [ ] Narrowphase convex/convex: **GJK** (Gilbert–Johnson–Keerthi) + **EPA** (Expanding Polytope) для глубины проникновения.
- [ ] Contact manifold clipping (reference: Box2D v3 / Bullet).
- [ ] 3D-кватернионы в интегрировании (см. Catto «Numerical Methods»).
- [ ] Constraints 3D: hinge, ball-socket, slider, cone twist, 6DOF.
- [ ] Sequential Impulse solver с warm-starting (как в Box2D/Bullet).
- [ ] Island detection и sleeping для больших сцен.
- [ ] Raycast / shapecast / volume query — API для gameplay и редактора.
- [ ] CCD (speculative contacts или sweep-based).

### Ресурсы
- **PhysX source** (https://github.com/NVIDIA-Omniverse/PhysX) — industrial reference.
- **Bullet Physics** (https://github.com/bulletphysics/bullet3) — более читаемый, классический reference.
- **Gino van den Bergen** — «A Collision Detection Library» (статьи про GJK).
- Erin Catto, «Numerical Methods» — https://box2d.org/publications/.
- Сайт **dyn4j** (Java, но концепции универсальны): http://www.dyn4j.org/.
- «Game Physics Pearls» (ред. Gino van den Bergen).

---

## Этап 6 — Продвинутая физика: мягкие тела, жидкости, разрушаемость

Это «финальная» фича-часть движка. Каждую можно делать отдельным подпроектом.

### 6.1 Мягкие тела (Soft Body / Cloth)
- [ ] Particle-based soft body (PBD — Position Based Dynamics).
- [ ] XPBD с ограничениями (современный стандарт, Matthias Müller/Macklin).
- [ ] Ткань (mesh of distance constraints), само-коллизии.
- [ ] Деформируемые объёмные тела (Tetrahedral mesh + FEM simplified).

### 6.2 Жидкости
- [ ] SPH (Smoothed Particle Hydrodynamics) — particle-based, простая интеграция с нашим движком частиц.
- [ ] Grid-based (PIC/FLIP) для больших объёмов воды (advanced).
- [ ] Surface reconstruction (marching cubes из частиц или mesh).

### 6.3 Разрушаемость (Fracture / Destruction)
- [ ] Voronoi-fracture (pre-fractured meshes) — база.
- [ ] Real-time fracture (Müller et al. «Real-Time Dynamic Fracture»).
- [ ] Convex decomposition разрушенных частей (V-HACD).
- [ ] Острова осколков с собственной физикой, частицы debris.
- [ ] Reference: **NVIDIA Blast** (https://github.com/NVIDIA-Omniverse/NVBlast), **Chaos Destruction** (Unreal Engine).

### Ресурсы (этап 6)
- Matthias Müller, Miles Macklin — статьи по PBD/XPBD на https://matthias-research.github.io/pages/.
- Position-Based Dynamics papers (Müller et al. 2007), XPBD (Macklin & Müller 2016).
- **PBRT** (https://www.pbr-book.org/) — для понимания объёмного рендера/полях.
- **Ten Minute Physics** (Matthias Müller) — https://matthias-research.github.io/pages/ten-minute-physics/.
- Fluids for games — Matthias Müller GDC talk «RealTime Fluids for Games» (2003, SPH).
- «Game Engine Gems 3» — главы про разрушения.
- NVIDIA FleX/Flex source for reference.

---

## Этап 7 — Интеграция движка и asset pipeline

Соединяем рендер + физику + ECS в цельное приложение.

### Подзадачи
- [ ] Сцена: сериализация в JSON (или binary), сохранение/загрузка.
- [ ] Asset pipeline: импорт glTF → внутреннее представление; менеджер ресурсов с refcounting.
- [ ] Scripting layer: встроить **Lua** (sol2) или **C# (.NET)** для геймплейного кода; reflection над компонентами ECS.
- [ ] Компоненты движка: MeshRenderer, RigidBody, Collider, Camera, Light, AudioSource.
- [ ] Аудио (опционально): `miniaudio` или `SoLoud`.
- [ ] Hot-reload shaders и assets.
- [ ] Лаунчер и конфигурация (engine.ini).
- [ ] Separation: editor build vs runtime build (чтобы редактор не тащил в релиз).

### Ресурсы
- glTF: спецификация https://khr.io/gltf, либы https://github.com/jkuhlmann/cgltf, https://github.com/syoyo/tinygltf.
- sol2 (Lua binding): https://github.com/ThePhD/sol2.
- miniaudio: https://github.com/mackron/miniaudio.
- **Game Engine Architecture** (Gregory) — главы про asset pipeline, scripting, reflection.

---

## Этап 8 — Встроенный 3D-редактор (immediate UI)

Встроенный редактор моделей, как в Godot/Unreal. Использует подсистемы движка (рендер, физику, ECS) — отдельное приложение поверх engine API.

### Подзадачи
- [ ] Базовый UI-каркас на Dear ImGui: dockable panels, menubar, toolbar.
- [ ] Viewport: 3D-камера (orbit/pan), gizmo (translate/rotate/scale), grid, snapping.
- [ ] Outliner (дерево объектов сцены), Properties (компоненты по reflection).
- [ ] Selection (single/multi), навигация по сцене (WASD + mouse).
- [ ] Undo/Redo stack (command pattern).
- [ ] Save/Load сцены и проекта.
- [ ] Инструменты моделирования (импортировано/написано с нуля):
  - [ ] Создание примитивов (cube, sphere, cylinder, plane).
  - [ ] Редактирование вершин/рёбер/граней (B-mesh-like или просто half-edge).
  - [ ] Extrude, inset, bevel, loop cut.
  - [ ] Boolean operations (CSG, опционально через `CGAL`).
  - [ ] Subdivision surface (Catmull-Clark).
  - [ ] Snapping, UV unwrap (минимальный).
- [ ] Material editor (shader parameters, texture slots, live preview).
- [ ] Собственный формат модели (`.tmesh`) + экспорт в glTF для совместимости.
- [ ] Bridge физики в редакторе: разместить коллайдеры, визуализировать AABB/joints.

### Ресурсы (этап 8)
- Dear ImGui: https://github.com/ocornut/imgui — изучить `imgui_demo.cpp` и примеры с multi-viewport.
- **ImGuizmo** для gizmo: https://github.com/Cedric Guillemet/ImGuizmo.
- **Godot editor source** — лучший reference для встроенного редактора: https://github.com/godotengine/godot.
- **Blender source** (модуль `bmesh`) — для алгоритмов моделирования: https://github.com/blender/blender.
- Half-edge data structure: https://www.flipcode.com/archives/The_Half-Edge_Data_Structure.shtml.
- **Compuphase** PolyPartition, **CGAL** — boolean/convex decomposition.
- **V-HACD** для convex decomposition: https://github.com/kmammou/v-hacd.
- Qt vs ImGui — выбрали ImGui (immediate mode, подвижный layout, легче интегрируется в движок).

---

## Этап 9 — Полировка и production-готовность

- [ ] Профиляция узких мест (Tracy, superluminal).
- [ ] Multithreading физики (параллельные острова, parallel solver).
- [ ] GPU-driven rendering (indirect draws, GPU culling) — на Vulkan-этапе.
- [ ] Editor ↔ engine IPC для live-preview.
- [ ] Платформы: Linux (основная), Windows (MinGW/MSVC).
- [ ] Документация API (Doxygen/Sphinx), примеры.
- [ ] Демо-сцены: разрушаемое здание, ткань, SPH-вода.

---

## Горизонтальные треки (можно вести параллельно)

| Трек | Ведёт | Зависимости |
|---|---|---|
| **Рендер** (этапы 1 → 4 → рендер-расширения 5/6) | интерфейс `IRenderer`, OpenGL/Vulkan | — |
| **Физика** (этапы 2 → 3 → 5 → 6) | интерфейс `IPhysicsWorld` | использует `IDebugDraw` из рендера |
| **Инструменты** (этапы 1 → 7 → 8) | editor, asset pipeline | зависит от обоих треков |

Договориться об интерфейсах (`IRenderer`, `IDebugDraw`, `IPhysicsWorld`, `IAssetManager`) **до** начала параллельной работы — иначе слияние треков будет болезненным.

---

## Канонический список книг (master reading list)

1. **Ian Millington — Game Physics Engine Development** — вход в физический движок.
2. **Christer Ericson — Real-Time Collision Detection** — коллизии.
3. **Erin Catto — GDC materials** — продвинутая физика контактов (https://box2d.org/publications/).
4. **Jason Gregory — Game Engine Architecture** — общая архитектура движка.
5. **Eric Lengyel — Foundations of Game Engine Development** (Vol 1 Mathematics, Vol 2 Rendering) — математика и рендер.
6. **Real-Time Rendering** (Akenine-Möller et al.) — bible по рендеру.
7. **Tomas Akenine-Möller, Eric Haines — Physically Based Rendering (PBRT)** — https://www.pbr-book.org/.
8. **Dirk Gregorius — GDC talks** по constraint solving.

## Open-source движки для reference-чтения

- **Box2D** (https://github.com/erincatto/box2d) — лучшее учебное чтение по 2D физике.
- **Bullet3** (https://github.com/bulletphysics/bullet3) — классический 3D физический движок.
- **PhysX** (https://github.com/NVIDIA-Omniverse/PhysX) — industrial reference.
- **Jolt Physics** (https://github.com/jrouwe/JoltPhysics) — современный многопоточный физический движок, отличное чтение.
- **Godot** (https://github.com/godotengine/godot) — best reference для встроенного редактора и общей архитектуры движка.
- **Hazel** (https://github.com/TheCherno/Hazel) — учебный движок, серии на YouTube.
- **Wicked Engine** (https://github.com/turanszkij/WickedEngine) — хороший reference по рендеру.
- **bgfx** (https://github.com/bkaradzic/bgfx) — для вдохновения по render API abstraction.
- **NVIDIA Blast** (https://github.com/NVIDIA-Omniverse/NVBlast) — разрушаемость.

## Сообщества

- Reddit r/gamedev, r/graphicsprogramming, r/cpp.
- GDC Vault (https://www.gdcvault.com/) — доклады по физике, рендеру, разрушениям.
- Discord: Vulkan, Dear ImGui, Box2D, Godot.
- Stack Exchange: https://gamedev.stackexchange.com/, https://computergraphics.stackexchange.com/.
- GameDev.ru — русскоязычный форум.

---

## Рекомендация по порядку прохождения

1. Этап 0 (инфраструктура) — **полностью**.
2. Этап 1 (тестовый стенд SFML) + Этап 2 (частицы) — параллельно, итеративно.
3. Этап 3 (2D rigid body) — закрепление физики.
4. Этап 4 (свой рендер, OpenGL) — вилка, на которой встречаются физика и графика.
5. Этап 5 (3D физика) + параллельное расширение рендера до 3D.
6. Этап 7 (asset pipeline + ECS-сцена) — параллельно с 5.
7. Этап 8 (редактор) — после стабильного 4/5, чтобы было что редактировать.
8. Этап 6 (soft body / fluids / destruction) — финальные фичи, последовательно.
9. Этап 9 — постоянная полировка.

Возвращаться к пройденным этапам и расширять (итеративный подход — как указано в `firstVersionPlan.md`).


