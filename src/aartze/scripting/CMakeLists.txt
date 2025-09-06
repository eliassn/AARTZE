# Ensure pybind11 chooses the same Python interpreter used to run the editor.
set(PYBIND11_FINDPYTHON ON CACHE BOOL "" FORCE)
if (WIN32)
  if (EXISTS ${CMAKE_SOURCE_DIR}/.venv312/Scripts/python.exe)
    set(Python3_EXECUTABLE ${CMAKE_SOURCE_DIR}/.venv312/Scripts/python.exe CACHE FILEPATH "" FORCE)
    set(Python_EXECUTABLE  ${CMAKE_SOURCE_DIR}/.venv312/Scripts/python.exe CACHE FILEPATH "" FORCE)
  elseif (EXISTS ${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe)
    set(Python3_EXECUTABLE ${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe CACHE FILEPATH "" FORCE)
    set(Python_EXECUTABLE  ${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe CACHE FILEPATH "" FORCE)
  endif()
endif()

# Resolve the actual Python we will use for building the extension
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
message(STATUS "AARTZE scripting: Python at ${Python3_EXECUTABLE} (v${Python3_VERSION})")
set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE} CACHE FILEPATH "" FORCE)

add_library(aartze_scripting STATIC PyBridge.cpp)
target_include_directories(aartze_scripting PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_SOURCE_DIR}/src)
target_compile_features(aartze_scripting PUBLIC cxx_std_17)
target_link_libraries(aartze_scripting PUBLIC aartze_core aartze_render pybind11::module)

# pybind11 module: aartzepy
find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(aartze_py MODULE PyBridge.cpp)
target_link_libraries(aartze_py PRIVATE aartze_core aartze_render aartze_input)
target_include_directories(aartze_py PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_SOURCE_DIR}/src)
if (WIN32)
  target_link_libraries(aartze_py PRIVATE OpenGL32)
endif()
set_target_properties(aartze_py PROPERTIES
  OUTPUT_NAME aartzepy
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
)
