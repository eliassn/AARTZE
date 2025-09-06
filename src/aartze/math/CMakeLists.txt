## Math is header-only for now → make it INTERFACE so no .lib is expected
add_library(aartze_math INTERFACE)
target_include_directories(aartze_math INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(aartze_math INTERFACE glm::glm)
