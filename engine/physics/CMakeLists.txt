# tml_physics — слой L4 (architecture.md §5.7).
#
# tml_physics_common: интерфейс IWorld2D + общие типы (header-only).
# tml_physics_particles: ParticleWorld : IWorld2D — минимальная заглушка
# для первой визуализации.

add_library(tml_physics_common INTERFACE)
target_include_directories(tml_physics_common INTERFACE include)
target_link_libraries(tml_physics_common INTERFACE tml_math tml_render_api tml_core)
tml_set_warnings(tml_physics_common)

add_library(tml_physics_particles STATIC src/particleWorld.cc)
target_include_directories(tml_physics_particles PUBLIC include)
target_link_libraries(tml_physics_particles
    PUBLIC
        tml_physics_common
    PRIVATE
        tml_core
)

tml_set_warnings(tml_physics_particles)
