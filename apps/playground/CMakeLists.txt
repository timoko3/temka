# temka_playground — визуальный отладчик физики (architecture.md §5.10, modulePlan §10).
# Этап-заглушка: окно, падающие частицы, без ввода.

add_executable(temka_playground main.cc)

target_link_libraries(temka_playground
    PRIVATE
        tml_engine
        tml_platform_raylib
        tml_render_raylib
        tml_physics_particles
        tml_core
)

tml_set_warnings(temka_playground)
