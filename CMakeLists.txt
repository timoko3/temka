# =============================================================================
# temka — корневой файл сборки (CMake)
#
# CMake — это ГЕНЕРАТОР сборочных файлов: он один раз читает CMakeLists.txt
# и создаёт «настоящую» сборочную систему (у нас — Makefile'ы для make).
#
# Базовый цикл работы (из корня репозитория):
#   cmake -S . -B build          # КОНФИГУРАЦИЯ: читает CMakeLists.txt,
#                                # создаёт каталог build/ со сгенерированными
#                                # Makefile'ами. Делается один раз, потом только
#                                # при изменении самих CMakeLists.txt.
#   cmake --build build          # СБОРКА: вызывает make внутри build/.
#                                # Именно эта команда выполняется при работе
#                                # над кодом — она пересобирает только
#                                # изменившиеся файлы.
#   ctest --test-dir build       # ТЕСТЫ: запускает все зарегистрированные
#                                # тесты и печатает сводку.
#
# Исходники НЕ копируются в build/ — там лежат только объектники и бинарники.
# Каталог build/ можно удалить в любой момент и пересоздать конфигурацией.
# =============================================================================

# Минимальная версия CMake. Если у кого-то CMake старше — конфигурация
# остановится сразу с понятной ошибкой, а не с загадочными сбоями дальше.
cmake_minimum_required(VERSION 3.20)

# Имя проекта, версия, языки. После этой строки существуют переменные
# PROJECT_NAME, PROJECT_SOURCE_DIR (корень репозитория) и т.п.
project(temka VERSION 0.1.0 LANGUAGES CXX)

# ----------------------------------------------------------------------------
# Стандарт языка
# ----------------------------------------------------------------------------
# Зафиксирован в organization/architecture.md §1.7 — C++17.
#   CXX_STANDARD          — сам стандарт;
#   CXX_STANDARD_REQUIRED — запрет «тихого» отката на более старый стандарт,
#                           если компилятор не умеет C++17 (будет ошибка);
#   CXX_EXTENSIONS OFF    — чистый ISO C++17 (-std=c++17, а не -std=gnu++17,
#                           без GNU-расширений).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------------------------------------------------------
# Опции проекта
# ----------------------------------------------------------------------------
# Переключатели сборки. Посмотреть все:  cmake -S . -B build -LH
# Задать при конфигурации:               cmake -S . -B build -DTML_SANITIZERS=ON
option(TML_BUILD_TESTS "Собирать тестовый раннер temka_tests" ON)
option(TML_SANITIZERS  "Собирать с AddressSanitizer + UndefinedBehaviorSanitizer" OFF)

# ----------------------------------------------------------------------------
# Общие настройки для ВСЕХ целей проекта
# ----------------------------------------------------------------------------

# Пишем build/compile_commands.json — базу команд компиляции. Её читают
# clangd/редакторы, чтобы показывать те же ошибки и подсказки, что видит
# компилятор при реальной сборке.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Бинарники всех целей складываем в build/bin/ — их легко найти и запустить:
#   ./build/bin/temka_tests
#   ./build/bin/temka_playground
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# cmake/ — каталог с нашими вспомогательными *.cmake-модулями. Добавляем его
# в путь поиска, чтобы include(Warnings) находил cmake/Warnings.cmake.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Warnings)

# ----------------------------------------------------------------------------
# Внешние зависимости
# ----------------------------------------------------------------------------
# Единственная внешняя библиотека первого этапа — raylib (бэкенд окна/рендера).
# Используем pkg-config, потому что raylib установлен в систему.
# Переменные RAYLIB_* используются в platform/ и render/ CMakeLists.txt.
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)

# ----------------------------------------------------------------------------
# Санитайзеры (включаются опцией TML_SANITIZERS=ON)
# ----------------------------------------------------------------------------
# ASan ловит порчу памяти (use-after-free, выход за границы), UBSan —
# неопределённое поведение (переполнения, невалидные приведения и т.п.).
# Флаги нужны и компилятору, и линкеру. Включаем глобально: санитайзер
# работает корректно, только если им собрано ВСЁ, что линкуется в бинарник.
#
# Отдельный каталог для санитайзер-сборки, чтобы не смешивать с обычной:
#   cmake -S . -B build-asan -DTML_SANITIZERS=ON && cmake --build build-asan
if(TML_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# ----------------------------------------------------------------------------
# Подключение модулей
# ----------------------------------------------------------------------------
# Каждый модуль движка — отдельный каталог со своим CMakeLists.txt, в котором
# описана своя ЦЕЛЬ (target). Здесь мы только перечисляем каталоги —
# «вход» в модуль происходит через add_subdirectory.
#
# Цели бывают трёх видов:
#   add_library(tml_core STATIC ...)      — обычная библиотека (есть .cpp);
#   add_library(tml_math INTERFACE)       — header-only: цель без компиляции,
#                                           несёт только include-пути
#                                           и зависимости;
#   add_executable(temka_playground ...)  — приложение.
#
# Зависимости между целями задаются ТОЛЬКО через target_link_libraries —
# CMake сам выстроит порядок сборки и прокинет include-пути «по цепочке».
# Глобальные include_directories() не используем (architecture.md §9).
#
# Правило слоёв (architecture.md §4): цель линкуется только с теми целями,
# которые перечислены в её строке «Зависит от». Нарушение = ошибка ревью.

# --- Этап 0: фундамент ---
# Строки подключения раскомментируются по мере написания модулей:
add_subdirectory(engine/core)         # tml_core:   лог, assert, Handle, Pool
add_subdirectory(engine/math)         # tml_math:   векторы/матрицы (header-only)
add_subdirectory(engine/ecs)          # tml_ecs:    сущности/компоненты (header-only)
add_subdirectory(engine/platform)     # tml_platform_api + tml_platform_raylib
add_subdirectory(engine/render)       # tml_render_api  + tml_render_raylib
add_subdirectory(engine/physics)      # tml_physics_common + tml_physics_particles
add_subdirectory(engine/engine)       # tml_engine: фасад, game loop

# --- Приложения ---
add_subdirectory(apps/playground)     # temka_playground: визуальный отладчик

# ----------------------------------------------------------------------------
# Тесты
# ----------------------------------------------------------------------------
# enable_testing() включает интеграцию с CTest: тестовый раннер регистрируется
# через add_test(), а `ctest --test-dir build` запускает всё сразу.
if(TML_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
