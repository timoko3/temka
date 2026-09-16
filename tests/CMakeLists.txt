# temka_tests — единый тестовый раннер движка (architecture.md §10).
#
# Каждый тест регистрируется макросом TM_TEST (tests/testFramework.h) и попадает
# в общий реестр статической инициализацией — отдельно «перечислять» тесты
# не нужно. По мере появления модулей сюда добавляются их test*.cpp
# и линкуются соответствующие библиотеки.
add_executable(temka_tests
    main.cpp
    core/testCore.cpp
    math/testMath.cpp
    ecs/testEcs.cpp
)

# Каталог tests/ — в include-путях, чтобы из тестов в подкаталогах
# (tests/core/..., tests/math/...) testFramework.h находился как
# #include "testFramework.h"
target_include_directories(temka_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(temka_tests PRIVATE
    tml_core
    tml_math
    tml_ecs
)

tml_set_warnings(temka_tests)

# Регистрация в CTest: `ctest --test-dir build` выполнит этот бинарник
# и проверит код возврата (0 = все тесты зелёные).
add_test(NAME temka_tests COMMAND temka_tests)
