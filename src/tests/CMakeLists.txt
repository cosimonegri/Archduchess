include(AddCatch2)

add_executable(archduchess_tests test.cpp)
target_link_libraries(archduchess_tests PRIVATE engine Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(archduchess_tests)
