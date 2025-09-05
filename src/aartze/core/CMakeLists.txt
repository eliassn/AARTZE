add_library(aartze_core STATIC Log.cpp)
target_include_directories(aartze_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_compile_features(aartze_core PUBLIC cxx_std_17)

