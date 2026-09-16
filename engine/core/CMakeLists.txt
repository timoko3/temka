# tml_core — фундамент движка (architecture.md §5.1): лог, assert, Handle, Pool.
# Слой L0 — ни от чего не зависит.
#
# Обычная статическая библиотека: есть .cc-файлы, компилируется один раз,
# линкуется всеми модулями, которым она нужна.
add_library(tml_core STATIC
    src/assert.cc
    src/log.cc
)

# PUBLIC include-путь: и сам модуль, и все цели, слинкованные с tml_core,
# видят заголовки как  #include "tml/core/log.hh"
target_include_directories(tml_core PUBLIC include)

# Строгие предупреждения из cmake/Warnings.cmake (-Wall -Wextra -Wpedantic -Werror)
tml_set_warnings(tml_core)
