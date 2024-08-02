include(AddCatch2)

add_executable(tests test.cpp)
target_link_libraries(tests PRIVATE engine Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)
