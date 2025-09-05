add_library(aartze_geometry STATIC HalfEdgeMesh.cpp BVH.cpp MeshOps.cpp)
target_include_directories(aartze_geometry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_compile_features(aartze_geometry PUBLIC cxx_std_17)
target_link_libraries(aartze_geometry PUBLIC aartze_core glm::glm)

