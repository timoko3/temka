# tml_ecs — Entity-Component-System (architecture.md §5.5).
# Header-only → INTERFACE-библиотека. Зависит от tml_core (Entity — хэндл-
# подобный uint64, TM_ASSERT, Handle). Сама реализация — в ecs.hh.
add_library(tml_ecs INTERFACE)

target_include_directories(tml_ecs INTERFACE include)
target_link_libraries(tml_ecs INTERFACE tml_core)

tml_set_warnings(tml_ecs)
