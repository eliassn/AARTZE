add_library(aartze_animation STATIC Rig.cpp)
target_include_directories(aartze_animation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_compile_features(aartze_animation PUBLIC cxx_std_17)
target_link_libraries(aartze_animation PUBLIC aartze_core ozz_animation::ozz_animation)

