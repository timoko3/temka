# План работы по модулям

> Детализация `architecture.md` до уровня «что делаем в каждом модуле и когда считаем готовым».
> Нумерация этапов соответствует `projectPlan.md` (§11 architecture.md).
> Порядок внутри этапа — сверху вниз; модули одного этапа можно вести параллельными треками.

Обозначения:
- **Этап** — когда модуль создаётся/основной объём работы.
- **Вход** — что должно быть готово до старта.
- **DoD** (definition of done) — критерии готовности модуля.
- **Тесты** — что попадает в `temka_tests` / визуальный smoke.

---

## 0. Инфраструктура сборки (не таргет, фундамент)

**Этап:** 0. **Вход:** пустой репозиторий.

- [ ] Корневой `CMakeLists.txt`: опции (`TML_BUILD_TESTS`, `TML_ENABLE_SANITIZERS`, `TML_WARNINGS_AS_ERRORS`), пресеты Debug / RelWithDebInfo / Sanitize / Analyze.
- [ ] `cmake/Warnings.cmake`, `cmake/Sanitizers.cmake`, `.clang-format`, `.clang-tidy`.
- [ ] Подключение extern: SFML, Dear ImGui (сборка как `SYSTEM`, изоляция include-путей).
- [ ] Скелет `temka_tests` + мини-фреймворк `tml_test` (`TM_TEST` / `TM_CHECK`, авторегистрация, ctest-интеграция).
- [ ] CI-скрипт (или локальный скрипт): configure → build → test в Debug и Sanitize.

**DoD:** пустое приложение собирается с `-Wall -Wextra -Werror`, тестовый раннер печатит 0 tests / 0 failures, ASan+UBSan проходят.

---

## 1. `tml_core` (L0)

**Этап:** 0. **Вход:** инфраструктура сборки. **Зависит от:** —.

### Задачи
- [ ] Логирование: `LogLevel`, каналы-теги, `log::message` printf-style (`__attribute__((format))`), макросы `TM_*`; вывод в stdout + опционально в файл; уровни глобально и по каналу.
- [ ] Assert: `TM_ASSERT` / `TM_ASSERT_MSG` — debug → `abort`, release → политика (лог + abort / лог).
- [ ] `Handle32` / `Handle<Tag>`: 8 байт, `isValid`, `operator==`.
- [ ] `Pool<T, Tag>`: поколенческий пул, `create/tryGet/destroy/size/forEach`, плотная итерация, переиспользование слотов.
- [ ] Профайлер (минимум): `prof::beginFrame/scope/event/dump`, RAII `ProfileScope`, экспорт Chrome Trace JSON. Thread-local стек областей.
- [ ] `UniquePtr`-алиас и место под будущие arena-аллокаторы (`makeUnique<T>(Arena&)` — заглушка API).

### DoD
- Пул переживает `destroy` + `create` без невалидных хэндлов; `tryGet` старым хэндлом возвращает `nullptr`.
- Лог не аллоцирует на каждый вызов больше одного форматирования; assert-политики переключаются сборкой.
- `prof::dump` открывается в `about:tracing`.

### Тесты
`core`: pool (создание/удаление/поколения/forEach), handle валидность, log уровни (перехват вывода), Chrome Trace JSON валиден.

---

## 2. `tml_math` (L0, header-only)

**Этап:** 0. **Вход:** —. **Зависит от:** —.

### Задачи
- [ ] `Vec2/3/4`, `Mat3/4` (column-major), `Quat` — полный набор операций: `+,-,*, dot, cross, length, normalize, lerp`.
- [ ] Матрицы: mul, `inverse`, `transpose`, `lookAt`, `ortho`, `perspective`.
- [ ] Кватернионы: `fromAxisAngle`, `toMat4`, `slerp`, normalize, умножение.
- [ ] `Transform` (`position/rotation/scale` + `toMat4`), `transformPoint/Vector`.
- [ ] Геометрия: `AABB` (`expand`, `intersects`), `Plane`, `Ray` + пересечения ray-AABB, ray-plane, ray-sphere.
- [ ] `constexpr`/`noexcept` где уместно; никаких внешних зависимостей.

### DoD
Все операции имеют unit-тесты с эталонами (вручную посчитанные значения + свойства: `inverse(M)*M ≈ I`, `|normalize(v)| ≈ 1`, `slerp` граничные случаи).

### Тесты
`math`: раздел на каждую структуру; сравнение float с эпсилоном.

---

## 3. `tml_platform_api` + `tml_platform_sfml` (L1)

**Этап:** 0. **Вход:** core, extern SFML.

### Задачи
- [ ] `WindowDesc`, enum `Key` / `Mouse` (полный набор, `Count` последним), `InputState` (keyDown/keyPressed/mouseDown/mousePos/mouseDelta/wheelDelta).
- [ ] `IPlatform` (интерфейс): `init/pollEvents/input/nativeWindowHandle/timeSeconds/requestClose`; глобальный доступ `platform()`.
- [ ] Реализация поверх SFML: перевод `sf::Event` → `InputState` + публикация в EventBus (события `WindowResized/WindowClosed/KeyPressed/MouseButtonPressed`).
- [ ] Семантика «нажато в этом кадре»: keyPressed живёт ровно один кадр.
- [ ] VSync-переключение, resizable, корректный `timeSeconds` (монотонный).

### DoD
Окно открывается, закрывается по крестику и `requestClose()`; InputState корректно отражает нажатия/дельты мыши; keyPressed сбрасывается каждый кадр.

### Тесты
Ручной smoke (окно + дамп InputState в лог); unit: маппинг SFML-кодов → `Key` таблицей.

---

## 4. `tml_events` (L2)

**Этап:** 0. **Вход:** core.

### Задачи
- [ ] `EventBus`: типизированные `publish/subscribe/dispatch/clear`; очередь с отложенной доставкой.
- [ ] Потокобезопасный `publish` (mutex или lock-free очередь — достаточно mutex на этапе 0); `dispatch` только на главном потоке.
- [ ] Подписки хранятся по `std::type_index`; доставка в порядке подписки.
- [ ] Базовые события платформы (структуры из §5.3 architecture.md).

### DoD
`publish` из worker-потока + `dispatch` на main работает без гонок (проверено TSan); события, опубликованные во время `dispatch`, доставляются в следующем кадре (нет реентерабельности).

### Тесты
`events`: подписка/отписка, порядок доставки, публикация из другого потока, dispatch-очередность.

---

## 5. `tml_jobs` (L2)

**Этап:** 0. **Вход:** core.

### Задачи
- [ ] `JobSystem`: пул потоков (`hw_concurrency − 1`, минимум 1), именованные потоки для профайлера.
- [ ] `parallelFor(count, minBatch, fn, dependency)`: разбиение на батчи, work-stealing, main помогает пулу при синхронном вызове.
- [ ] `run(fn, dependency)` — одиночная задача; `Counter` — счётчик зависимостей.
- [ ] `waitFor(Counter&)` — активное ожидание с work-stealing (не блокирующий sleep).
- [ ] `threadIndex()` внутри задачи.
- [ ] Обкатка API: `parallelFor` в тестах + (позже) частицы и asset loader — без изменения API.

### DoD
`parallelFor(1'000'000)` во всех комбинациях потоков даёт ровно однократное покрытие диапазона; TSan чистый; вложенный `parallelFor` не дедлокится.

### Тесты
`jobs`: корректность покрытия, суммы по батчам, зависимости (задача B ждёт Counter A), TSan-прогон.

---

## 6. `tml_ecs` (L2, header-only)

**Этап:** 0. **Вход:** core.

### Задачи
- [ ] `Entity = uint64_t` (индекс + поколение); `Registry::create/destroy/valid`.
- [ ] Пулы компонентов: плотный массив (SoA-совместимый layout) + sparse-таблица сущностей; `emplace/getOrEmplace/tryGet/has/remove/clear`.
- [ ] `each<C...>(fn)` — итерация по пересечению (движение по самому редкому компоненту).
- [ ] Сигналы `onConstruct<C>/onDestroy<C>` — основа sync-систем сцены.
- [ ] Переиспользование слотов, инкремент поколения при `destroy`; инвалидация старых `Entity`.
- [ ] Никаких зависимостей кроме core; аллокации амортизированы.

### DoD
Уничтожение сущности → старый `Entity` невалиден, `tryGet` по нему `nullptr`; `each` по двум компонентам обходит ровно пересечение; сигналы вызываются ровно один раз на событие.

### Тесты
`ecs`: CRUD, поколения, each по 1/2/3 компонентам, сигналы, remove во время итерации — безопасно (или задокументировано как UB).

---

## 7. `tml_render_api` (L3, header-only)

**Этап:** 1 (создание), 4 (расширение DrawList). **Вход:** math.

### Задачи
- [ ] `Color`, `Camera` (+ фабрики `ortho2D`, `perspective`, `frustumAABB` для отсечения).
- [ ] `IDebugDraw`: полный контракт из §5.6 — point/line/polyline/circle/aabb/sphere/triad/arrow/text, `begin/end`, `pushLayer/popLayer`.
- [ ] `IRenderer`: `init/shutdown/beginFrame/endFrame/debug/backendName`; фабрика `createRenderer()` — в библиотеке бэкенда, НЕ в api.
- [ ] Зафиксировать правило: 2D = 3D с `z = 0`; никакого `IDebugDraw2D`.
- [ ] Комментарий-маячок о расширении этапа 4 (`draws()`, `device()`) — не добавлять сейчас.

### DoD
Заголовочный контракт компилируется в изоляции (тест-цель включает только `render/IDebugDraw.h` + `Camera.h`); инклуды физики/сцены не видят бэкендов (проверяется include-тестом / clang-tidy).

### Тесты
`render_api`: Camera-фабрики (ortho2D/perspective — эталонные матрицы), include-изоляция (compile-check: физика + только api-заголовки).

---

## 8. `tml_render_sfml` (L3)

**Этап:** 1. **Вход:** render_api, platform_sfml, extern SFML.

### Задачи
- [ ] `createRenderer()` → `SfmlRenderer : IRenderer`; `init(IPlatform&, WindowDesc&)` — рендер в SFML-окно (sf::RenderTarget / вершины).
- [ ] `DebugDrawSFML : IDebugDraw`: батчинг всех примитивов в вершинные массивы; `line/polyline` — `sf::Lines/LinesStrip`; `circle` — сегментированный полигон; `aabb/sphere/triad/arrow` — линии.
- [ ] `text` (этап 1.5): sf::Text поверх батчей (не батчится — отдельный проход).
- [ ] Слои `pushLayer/popLayer` — стек отдельных батч-наборов, рендер по порядку.
- [ ] Отсечение по `frustumAABB` камеры (базово).
- [ ] ImGui-оверлей: интеграция Dear ImGui + SFML backend; отдельный слой поверх debug-графики.

### DoD
Все примитивы `IDebugDraw` видимы и корректны; 100k линий между `begin/end` не просаживают кадр ниже 60 FPS (в режиме debug-стенда); смена камеры — без пересоздания бэкенда.

### Тесты
Визуальный smoke в playground: сетка, триад, круги, AABB, текст, слои.

---

## 9. `tml_engine` (L5)

**Этап:** 1. **Вход:** platform, events, jobs, render_api.

### Задачи
- [ ] `EngineConfig` (window, fixedDt, maxStepsPerFrame, logLevel).
- [ ] `Engine::run(Layer*, size_t)` — канонический цикл «Fix Your Timestep!» (§5.10): pollEvents → dispatch → accumulator-петля fixedUpdate → alpha → onRender.
- [ ] `Layer`: `onAttach/onDetach/onEvent/onFixedUpdate/onUpdate/onRender`.
- [ ] `Engine::IModule` + `addModule` — регистрация модулей (физический мир и т.п.) без правки Engine.
- [ ] Доступ подсистем: `renderer()/jobs()/events()/input()`.
- [ ] `maxStepsPerFrame` — защита от спирали смерти; deltaTime клампится.
- [ ] Инициализация/уничтожение по порядку зависимостей; requestClose из слоя/события.

### DoD
Игровой цикл: при 144 Гц мониторе и fixedDt = 1/60 количество `onFixedUpdate` ≈ 60/сек (проверяется логом), рендер каждый кадр; при ресайзе окна события `WindowResized` доходят до слоёв; паника в слое не оставляет висших потоков.

### Тесты
Unit: accumulator-логика (фиктивные тайминги — таблица случаев). Smoke: playground.

---

## 10. `temka_playground` (L6)

**Этап:** 1–3 (растёт вместе с физикой). **Вход:** engine, render_sfml.

### Задачи
- [ ] Этап 1: окно через Engine, слой DebugLayer — сетка, триад, демо-примитивы; камера 2D pan/zoom (WASD + wheel); FPS-счётчик.
- [ ] Этап 1.5: ImGui-оверлей — выбор демо-сцены, слайдеры параметров, статистика (объекты/контакты/шаги/время шага).
- [ ] Этап 2: демо-сцены частиц (фонтан, пружинная ткань, гравитационные колодцы) — визуальный отладчик интеграторов.
- [ ] Этап 3: демо-сцены rigid2d (стек боксов, домино, джойнты, pyramid stress-test); спавн тел мышью; визуализация контактов/AABB/спящих тел.
- [ ] Пауза/step-once/slow-motion — обязательные кнопки отладчика.
- [ ] Каждая демо-сцена = функция `registerScene(name, factory)` — расширение без правки каркаса.

### DoD
Smoke-критерий из §10 architecture.md: демо-сцена запускается, не падает, статистика объектов/контактов совпадает с эталоном запуска.

---

## 11. `tml_physics_common` (L4)

**Этап:** 2. **Вход:** math, render_api (только заголовки).

### Задачи
- [ ] `BodyId`, `BodyDesc2D` (POD, расширяемая), `RaycastHit2D`, `ContactEvent2D`.
- [ ] `IWorld2D` — полный интерфейс из §5.7; зафиксировать форму (её зеркалит будущий `IWorld3D`).
- [ ] Интеграторы: `ExplicitEuler / SemiImplicitEuler / Verlet / RK4` — как отдельные функции над state-структурой, сравнимые независимо от мира.
- [ ] Хелперы: аккумулятор fixed timestep (если решено вынести из engine), интерполяция состояний.
- [ ] Никакого времени внутри: `step(fixedDt)` — единственный источник времени (трактат №4).

### DoD
Все 4 интегратора проходят тесты на эталонных ODE (свободное падение, гармонический осциллятор): порядок точности соответствует теории; интерфейс компилируется без знания о реализациях.

### Тесты
`physics_common`: интеграторы — сходимость (halving step → ошибка падает в ожидаемой степени), энергия осциллятора (Euler расходится, semi-implicit — консервативен по амплитуде).

---

## 12. `tml_physics_particles` (L4)

**Этап:** 2. **Вход:** physics_common. **Реализует:** `ParticleWorld : IWorld2D`.

### Задачи
- [ ] Частица: position, velocity, force accumulator, invMass; хранение в `Pool`/плотных массивах.
- [ ] Поля сил: гравитация, drag (linear/quadratic), пружина, притяжение/отталкивание (генераторы силы — стратегия).
- [ ] Distance constraints (релаксация) — база soft body.
- [ ] Контакты с границами мира (отскок с restitution).
- [ ] `step(fixedDt)`: очистка сил → генераторы → интегратор → constraints → контакты. Выбор интегратора — параметром.
- [ ] `interpolatedPosition(id, alpha)` — хранение prevPosition.
- [ ] `debugDraw`: точки, вектора скорости, связи-пружины.
- [ ] Расширяющие методы: `addSpring(a, b, k)`, `addForceField(...)` — вне `IWorld2D`, по правилу §5.7.
- [ ] Обкатка `parallelFor` на интеграции позиций (опционально, API уже готов).

### DoD
Golden replay: фиксированная сцена, побайтовое сравнение траекторий float; сравнение интеграторов визуально в playground; 100k частиц — интерактивный FPS в debug-рендере.

### Тесты
`physics_particles`: свободное падение (аналитика), пружина (период), restitution границ, golden replay.

---

## 13. `tml_physics_rigid2d` (L4)

**Этап:** 3. **Вход:** physics_common, particles (интеграторы). **Реализует:** `RigidWorld2D : IWorld2D`.

### Задачи (порядок = порядок риска)
- [ ] Rigid body: position, angle, velocity, angularVelocity, invMass, invInertia (по форме); static (mass=0).
- [ ] Формы: circle, box (потом polygon); вычисление инерции и AABB.
- [ ] Broadphase: uniform grid → сравнить с sweep-and-prune и dynamic AABB-tree (эксперимент в playground, фиксируем лучший).
- [ ] Narrowphase: circle/circle, circle/box, box/box — SAT; контактные точки и нормали.
- [ ] Sequential impulse solver (Erin Catto): warm-starting, bias/baumgarte, iteration count в конфиге.
- [ ] Трение (Coulomb, ограничения по касательной) и restitution.
- [ ] Джойнты: distance, point-to-point (revolute), weld, hinge — через constraint-уравнения в общий solver.
- [ ] Sleeping: порог скорости/времени; острова (union-find); пробуждение по контакту.
- [ ] `IContactListener` + `ContactEvent2D` (begin/end).
- [ ] Raycast; `setTransform` (телепорт).
- [ ] Базовый CCD для быстрых тел (speculative или clamp скорости — решить по результату).
- [ ] `debugDraw`: формы, AABB, нормали контактов, спящие — другим цветом, джойнты.
- [ ] Кодируется сразу без скрытых глобалов (трактат №5) — параллелизация на этапе 5+ без рефакторинга.

### DoD
Тесты Box2D-типа: стек из 10 боксов не «взрывается» и не проседает; маятник на distance joint сохраняет энергию в допуске; resting contact стабилен при 1000 шагов; golden replay побайтово.

### Тесты
`physics_rigid2d`: stacked boxes, restitution/freedom аналитические случаи, island/sleeping, raycast, golden replay. Stress: 1000 боксов в пирамиде — критерий по времени шага.

---

## 14. `tml_scene` (L4)

**Этап:** 2–3 (базис), 7 (сериализация). **Вход:** ecs, physics_common, render_api.

### Задачи
- [ ] Компоненты: `TransformComponent`, `ParentComponent` (иерархия local/world), `RigidBody2D {BodyId, BodyDesc2D}`, `DebugDrawComponent`.
- [ ] `Scene(phys::IWorld2D&)` — сцена НЕ владеет миром (подмена ParticleWorld ↔ RigidWorld2D без правок).
- [ ] Sync-система: `onConstruct<RigidBody2D>` → `world.createBody(desc)`; `onDestroy` → `world.destroyBody(body)`.
- [ ] `fixedUpdate(fixedDt)`: применить правки desc → `world.step` → обратно `position → TransformComponent`.
- [ ] `render(renderer, alpha)`: debugDraw мира + `DebugDrawComponent` (AABB и др.).
- [ ] Этап 7: сериализация `save/load(path)` — состав сущностей + descs (формат нестабилен до этапа 7, потом — версионирование).
- [ ] `RigidBody2D::body` невалиден до создания sync-системой — задокументировать контракт.

### DoD
Создание/уничтожение сущностей с `RigidBody2D` в рантайме не течёт и не рассинхронизирует хэндлы; подмена мира меняет поведение сцены без перекомпиляции; save→load восстанавливает сцену побайтово (после этапа 7).

### Тесты
`scene`: sync construct/destroy, порядок fixedUpdate, иерархия трансформов, roundtrip сериализации.

---

## 15. `tml_render_gl` (L3) — этап 4

**Этап:** 4. **Вход:** render_api, platform (nativeWindowHandle), extern GLEW/GLAD (решить).

### Задачи
- [ ] GL-контекст поверх `nativeWindowHandle()` (SFML даёт контекст, либо отдельный).
- [ ] `GlRenderer : IRenderer` + `DebugDrawGL : IDebugDraw` — те же примитивы, вершинные буферы, один шейдер линий/точек.
- [ ] VAO/VBO/IBO, uniform-буферы (viewProj), GLSL-шейдеры.
- [ ] Расширение контракта `render_api` (отдельным коммитом-предложением!): `DrawList` — меши, материалы; `IRenderDevice` — буферы/шейдеры/текстуры.
- [ ] Загрузка мешей glTF (cgltf/tinygltf), текстуры (stb_image).
- [ ] Forward rendering: направленный + точечные источники, ambient; материал с параметрами; shadow map.
- [ ] Камеры 3D: perspective, orbit, FPS.
- [ ] Батчинг и отсечение debug-графики по frustumAABB.
- [ ] Playground/editor переключают бэкенд `sfml ↔ gl` заменой `createRenderer()` — единственная правка.

### DoD
Debug-графика идентична SFML-бэкену (визуальное сравнение); glTF-меш Дэмиена с текстурой и светом рендерится; существующая физика и сцена работают без правок (проверка трактата №1).

### Тесты
Визуальный smoke: та же демо-сцена на sfml и gl — совпадение статистики; unit: загрузчик glTF (кол-во мешей/вершин эталонного файла).

---

## 16. `tml_render_vk` (L3) — поздняя часть этапа 4

**Этап:** 4 (конец). **Вход:** render_gl (референс поведения), render_api.

### Задачи
- [ ] Instance/device/swapchain по `vulkan-tutorial.com` → `vkguide.dev`.
- [ ] `VkRenderer : IRenderer`, `DebugDrawVk : IDebugDraw` — render graph, descriptor sets.
- [ ] Паритет с GL-бэкендом по DebugDraw; DrawList-рендер — по образцу GL.
- [ ] Перенос шейдеров GLSL → SPIR-V.

### DoD
Три бэкенда (`sfml/gl/vk`) проходят один и тот же smoke-набор playground.

---

## 17. `tml_physics_rigid3d` (L4) — этап 5

**Этап:** 5. **Вход:** rigid2d (solver, islands), jobs.

### Задачи
- [ ] `IWorld3D` — зеркальная форма `IWorld2D` на Vec3/Quat (закреплено в открытых вопросах §12).
- [ ] Формы: sphere, box, capsule, convex hull, compound (triangle mesh — позже).
- [ ] Broadphase: dynamic AABB tree (BVH) + sweep-and-prune.
- [ ] Narrowphase: **GJK** + **EPA** (глубина проникновения), contact manifold clipping.
- [ ] Интегрирование кватернионов (Catto «Numerical Methods»).
- [ ] 3D-джойнты: ball-socket, hinge, slider, cone twist, 6DOF — поверх того же sequential impulse.
- [ ] Острова + sleeping + **параллельные острова через jobs** (`run` + Counter — зачем API готовился с этапа 0).
- [ ] Raycast / shapecast / volume query; CCD (speculative contacts).
- [ ] `debugDraw`: сферы/боксы/капсулы, contact manifold, GJK-отладка (симплексы).

### DoD
Стек 3D-боксов стабилен; GJK проходит тест-векторные случаи (известные дистаннии/точки); параллельные острова дают ускорение ≥ 2x на 4+ ядрах на сцене из разнесённых куч; TSan чистый; golden replay 3D.

### Тесты
`physics_rigid3d`: GJK/EPA на эталонных парах фигур, джойнты (аналитические случаи), параллельные острова (детерминизм результата при любом числе потоков).

---

## 18. `tml_physics_soft` (L4) — этап 6

**Этап:** 6. Три подпроекта, каждый — отдельная веха.

### 18.1 XPBD / мягкие тела / ткань
- [ ] `SoftWorld` — параллельный мир (объединяется с rigid на уровне сцены, не внутри).
- [ ] XPBD: distance/bending/volume constraints, substeps; cloth из distance constraints.
- [ ] Деформируемые объёмные тела (tetra mesh, упрощённый FEM).
- [ ] Само-коллизии ткани (пространственный хеш).
- [ ] Контакты soft ↔ rigid (через тот же IContactListener-контур).

### 18.2 SPH-жидкости
- [ ] SPH на базе particles-инфраструктуры: kernels, density/pressure, вязкость.
- [ ] Пространственный хеш соседей; вариант PBF (position-based fluids) — если SPH нестабилен.
- [ ] Surface reconstruction (marching cubes) — через render_api-расширение.

### 18.3 Разрушаемость
- [ ] Voronoi pre-fracture; осколки как rigid-тела (birth из активации sleeping-островов).
- [ ] Convex decomposition (V-HACD-подход, свой код или упрощение).
- [ ] Real-time fracture (Müller) — по достижении стабильности; частицы debris.

**DoD (этап 6):** ткань 32×32 падает на сферу без взрывов; SPH-бассейн 10k частиц интерактивен; voronoi-разрушение стены воспроизводимо (golden replay). Каждый подпроект — демо-сцена в playground.

---

## 19. `tml_assets` (L4/L7) — этап 7

**Этап:** 7. **Вход:** core (Pool, Handle), jobs (асинхронный IO), ecs.

### Задачи
- [ ] `AssetId`, `AssetManager::load/get<T>/update`; refcounting; выгрузка неиспользуемых.
- [ ] Реестр загрузчиков по расширению (gltf-меш, текстура, шейдер, .tmesh) — новый формат = новый загрузчик без правки менеджера.
- [ ] Асинхронная загрузка через jobs; `update()` достраивает результаты на main.
- [ ] Hot-reload шейдеров и ресурсов (по timestamp / явной команде).
- [ ] Версионирование формата сцен (магик-байт + версия).

### DoD
Загрузка 100 мешей не блокирует кадр (прогресс в статистике); повторный `load` того же пути возвращает тот же AssetId; hot-reload шейдера виден без рестарта.

### Тесты
`assets`: refcount, дедупликация путей, фейковый загрузчик + jobs-очередь, версионирование (старый файл → понятная ошибка).

---

## 20. `tml_script` (L4/L7) — этап 7

**Этап:** 7. **Вход:** scene (компоненты), reflection-таблицы.

- [ ] Reflection-таблицы компонентов (имя → поля/desc).
- [ ] Встраивание Lua (sol2) или отсрочка решения — по итогам этапа 7.
- [ ] API скрипта: доступ к сущностям, подписка на события, fixedUpdate-хук.

**DoD:** скрипт создаёт сущность с RigidBody2D и реагирует на контакт — без C++-правок.

---

## 21. `temka_editor` (L6) — этап 8

**Этап:** 8. **Вход:** engine, scene, render_gl, assets, ImGui.

### Задачи
- [ ] Каркас: dockable panels, menubar, toolbar на ImGui; приложение = набор слоёв Engine.
- [ ] Viewport: orbit/pan/FPS-камера, gizmo (translate/rotate/scale, ImGuizmo-подход), grid, snapping.
- [ ] Outliner (дерево сцены), Properties (по reflection-таблицам).
- [ ] Selection single/multi; навигация WASD.
- [ ] Undo/Redo — command pattern; каждая мутация сцены — команда.
- [ ] Save/Load проекта/сцены; разделение editor/runtime build.
- [ ] Инструменты моделирования: примитивы → вершины/рёбра/грани (half-edge) → extrude/inset/bevel/loop cut → boolean (опц.) → subdivision (Catmull-Clark).
- [ ] Material editor (параметры, текстуры, live preview).
- [ ] Формат `.tmesh` + экспорт glTF.
- [ ] Bridge физики: размещение коллайдеров/джойнтов, визуализация AABB/contacts.

### DoD
Полный цикл: создать куб → отредактировать → назначить материал и RigidBody → запустить симуляцию в viewport → undo → сохранить → загрузить → воспроизвести идентично.

---

## 22. Сквозные горизонтали (везде, постоянно)

- [ ] Профилирование: `ProfileScope` во всех hot-path (step, debugDraw, dispatch) — с этапа 1.
- [ ] Трактаты §6 architecture.md: каждое ревью проверяет включения и границы (include-тест + ревью-чеклист).
- [ ] Golden replay-библиотека: накапливаем эталонные траектории по мере роста физики.
- [ ] Этап 9: многопоточность solver, GPU-driven rendering (vk), IPC editor↔engine, Windows/MinGW, документация API, демо-сцены (разрушаемое здание, ткань, вода).

---

## Матрица «модуль → этап» (сводка)

| Модуль | Этап создания | Основной рост | Параллельный трек |
|---|---|---|---|
| инфраструктура, core, math, platform, events, jobs, ecs | 0 | 0 | общий |
| render_api, render_sfml, engine, playground | 1 | 1 | рендер |
| physics_common, particles, scene (базис) | 2 | 2 | физика |
| physics_rigid2d, scene (sync) | 3 | 3 | физика |
| render_gl, render_vk | 4 | 4 | рендер |
| physics_rigid3d | 5 | 5 | физика |
| physics_soft | 6 | 6 | физика |
| assets, script, scene (сериализация) | 7 | 7 | инструменты |
| editor | 8 | 8 | инструменты |
| профайлер, MT-физика, IPC | 9 | 9 | общий |
