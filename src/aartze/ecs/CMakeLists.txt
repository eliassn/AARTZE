add_library(aartze_ecs STATIC Scene.cpp)
target_include_directories(aartze_ecs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_compile_features(aartze_ecs PUBLIC cxx_std_17)
target_link_libraries(aartze_ecs PUBLIC aartze_core EnTT::EnTT)

