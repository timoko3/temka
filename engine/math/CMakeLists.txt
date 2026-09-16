# tml_math — математика (architecture.md §5.2): векторы, матрицы, AABB.
# Header-only → INTERFACE-библиотека: компилировать нечего, цель несёт только
# include-путь и зависимость от tml_core (TM_ASSERT в Mat4::inverse).
add_library(tml_math INTERFACE)

# INTERFACE include-путь: потребители видят #include "tml/math/vec2.hh"
target_include_directories(tml_math INTERFACE include)

# Зависимости (L0): только tml_core
target_link_libraries(tml_math INTERFACE tml_core)

# Флаги для INTERFACE лезут через отдельную ветку tml_set_warnings
tml_set_warnings(tml_math)
