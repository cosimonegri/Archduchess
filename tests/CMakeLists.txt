include(AddCatch2)

add_executable(tests_archduchess test_main.cpp)
target_link_libraries(tests_archduchess PRIVATE
    engine
    Catch2::Catch2WithMain
    Threads::Threads
)
target_compile_definitions(tests_archduchess PRIVATE
    TEST_SUITE_DIR="${CMAKE_SOURCE_DIR}/testsuites"
)

include(CTest)
include(Catch)
catch_discover_tests(tests_archduchess)
