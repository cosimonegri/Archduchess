include(AddCatch2)

enable_testing()

add_executable(tests_archduchess test_main.cpp)
target_link_libraries(tests_archduchess PRIVATE engine Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests_archduchess)
