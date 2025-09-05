add_library(aartze_import STATIC Importer.cpp AssimpImporter.cpp)
target_include_directories(aartze_import PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_compile_features(aartze_import PUBLIC cxx_std_17)
target_link_libraries(aartze_import PUBLIC aartze_core nlohmann_json::nlohmann_json assimp::assimp)

