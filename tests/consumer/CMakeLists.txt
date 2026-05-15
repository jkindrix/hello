# A minimal downstream consumer that finds the installed hello package and
# links against hello::hello. Built by the install-consumer CI job to prove
# that the install/export pipeline produces a working find_package() result.

cmake_minimum_required(VERSION 3.20)
project(hello_consumer LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(hello 1.0 REQUIRED)

add_executable(consumer consumer.c)
target_link_libraries(consumer PRIVATE hello::hello)

enable_testing()
add_test(NAME consumer_smoke COMMAND consumer)
set_tests_properties(consumer_smoke PROPERTIES
    PASS_REGULAR_EXPRESSION "Hello, Consumer!"
)
