# =============================================================================
# Warnings.cmake — единая точка настройки предупреждений компилятора.
#
# Использование (в CMakeLists.txt любого нашего модуля):
#
#   add_library(tml_core ...)
#   tml_set_warnings(tml_core)
#
# Все наши цели собираются со строгими предупреждениями, причём любое
# предупреждение = ошибка сборки (-Werror). Зафиксировано в architecture.md §9.
# ВНЕШНИЕ библиотеки (raylib и др.) этой функцией НЕ обрабатываются.
# =============================================================================

# Флаги:
#   -Wall       — базовый набор предупреждений (несмотря на имя, не «все»);
#   -Wextra     — дополнительные проверки (неиспользуемые параметры и т.п.);
#   -Wpedantic  — строгое соответствие ISO C++ (без компиляторных расширений);
#   -Werror     — любое предупреждение считается ошибкой компиляции.
function(tml_set_warnings target)
    # Тип цели важен: header-only (INTERFACE) библиотеки сами не компилируются,
    # поэтому флаги им передаются с ключом INTERFACE и «протекают» в те цели,
    # которые их подключают. Обычным целям флаги даются с PRIVATE —
    # только для их собственной компиляции.
    get_target_property(target_type ${target} TYPE)

    if(target_type STREQUAL "INTERFACE_LIBRARY")
        target_compile_options(${target} INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
        )
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
        )
    endif()
endfunction()
