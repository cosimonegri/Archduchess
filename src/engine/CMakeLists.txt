file (GLOB_RECURSE Engine_SOURCES CONFIGURE_DEPENDS "*.cpp")
file (GLOB_RECURSE Engine_HEADERS CONFIGURE_DEPENDS "*.hpp")

set (Engine_INCLUDE_DIRS "")
foreach (_headerFile ${Engine_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND Engine_INCLUDE_DIRS ${_dir})
endforeach()
list (REMOVE_DUPLICATES Engine_INCLUDE_DIRS)

add_library(engine ${Engine_SOURCES})
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
