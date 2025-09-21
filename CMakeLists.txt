cmake_minimum_required(VERSION 3.30)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(prelude)

project(archduchess VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


# --- Create engine library containing all source files except main.cpp ---
file(GLOB engine_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM engine_SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

add_library(engine ${engine_SOURCES})
target_include_directories(engine PUBLIC ${CMAKE_SOURCE_DIR}/src)
# -------------------------------------------------------------------------


# --- Crate main archduchess executable ---
add_executable(archduchess src/main.cpp)
target_link_libraries(archduchess PRIVATE engine)

target_compile_options(archduchess PRIVATE
    $<$<CONFIG:Debug>:-O0 -g -Wall>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
    $<$<CONFIG:Profile>:-no-pie -DNDEBUG -pg>
)
# -----------------------------------------


add_subdirectory(tests)
