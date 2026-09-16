# tml_platform — слой L1 (architecture.md §5.3).
#
# tml_platform_api: header-only интерфейсы (IPlatform, WindowDesc, InputState).
# tml_platform_raylib: реализация поверх raylib. Зависит от tml_platform_api
# и линкуется с raylib.

add_library(tml_platform_api INTERFACE)
target_include_directories(tml_platform_api INTERFACE include)
target_link_libraries(tml_platform_api INTERFACE tml_math)
tml_set_warnings(tml_platform_api)

add_library(tml_platform_raylib STATIC src/platformRaylib.cc)
target_include_directories(tml_platform_raylib PUBLIC include)

target_link_libraries(tml_platform_raylib
    PUBLIC
        tml_platform_api
        tml_core
    PRIVATE
        ${RAYLIB_LIBRARIES}
)

target_include_directories(tml_platform_raylib SYSTEM PRIVATE ${RAYLIB_INCLUDE_DIRS})
target_link_directories(tml_platform_raylib PRIVATE ${RAYLIB_LIBRARY_DIRS})
target_compile_options(tml_platform_raylib PRIVATE ${RAYLIB_CFLAGS_OTHER})

tml_set_warnings(tml_platform_raylib)
