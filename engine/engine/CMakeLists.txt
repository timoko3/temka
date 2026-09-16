# tml_engine — фасад и game loop (architecture.md §5.10).
# Зависит только от API platform/render, не от их бэкендов; бэкенды
# подключаются исполняемым приложением (playground/editor).

add_library(tml_engine STATIC src/engine.cc)
target_include_directories(tml_engine PUBLIC include)

target_link_libraries(tml_engine
    PUBLIC
        tml_platform_api
        tml_render_api
    PRIVATE
        tml_core
)

tml_set_warnings(tml_engine)
