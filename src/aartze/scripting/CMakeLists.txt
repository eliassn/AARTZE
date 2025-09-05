add_library(aartze_scripting STATIC PyBridge.cpp)
target_include_directories(aartze_scripting PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_compile_features(aartze_scripting PUBLIC cxx_std_17)
target_link_libraries(aartze_scripting PUBLIC aartze_core pybind11::module)

# pybind11 module: aartze (minimal)
pybind11_add_module(aartze_py MODULE PyBridge.cpp)
target_link_libraries(aartze_py PRIVATE aartze_core)
if (WIN32)
  target_link_libraries(aartze_py PRIVATE OpenGL32)
endif()
set_target_properties(aartze_py PROPERTIES
  OUTPUT_NAME aartze
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
)
