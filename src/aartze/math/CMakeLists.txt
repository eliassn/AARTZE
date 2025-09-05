add_library(aartze_math STATIC Math.h)
target_include_directories(aartze_math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_compile_features(aartze_math PUBLIC cxx_std_17)
target_link_libraries(aartze_math PUBLIC glm::glm)

