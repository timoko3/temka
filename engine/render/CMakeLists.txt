# tml_render — слой L3 (architecture.md §5.6).
#
# tml_render_api: header-only контракт (Color, Camera, IDebugDraw, IRenderer).
# tml_render_raylib: реализация поверх raylib. Зависит от tml_platform_api,
# потому что IRenderer::init принимает IPlatform&.

add_library(tml_render_api INTERFACE)
target_include_directories(tml_render_api INTERFACE include)
target_link_libraries(tml_render_api INTERFACE tml_math tml_core)
tml_set_warnings(tml_render_api)

add_library(tml_render_raylib STATIC
    src/rendererRaylib.cc
    src/debugDrawRaylib.cc
)
target_include_directories(tml_render_raylib PUBLIC include)

# Приватный заголовок debugDrawRaylib.hh лежит в src/
target_include_directories(tml_render_raylib PRIVATE src)

target_link_libraries(tml_render_raylib
    PUBLIC
        tml_render_api
        tml_platform_api
        tml_core
    PRIVATE
        ${RAYLIB_LIBRARIES}
)

target_include_directories(tml_render_raylib SYSTEM PRIVATE ${RAYLIB_INCLUDE_DIRS})
target_link_directories(tml_render_raylib PRIVATE ${RAYLIB_LIBRARY_DIRS})
target_compile_options(tml_render_raylib PRIVATE ${RAYLIB_CFLAGS_OTHER})

tml_set_warnings(tml_render_raylib)
