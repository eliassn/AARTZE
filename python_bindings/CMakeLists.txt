cmake_minimum_required(VERSION 3.16)

# Ensure pybind11 uses modern FindPython to match our chosen interpreter
set(PYBIND11_FINDPYTHON ON CACHE BOOL "" FORCE)

# Prefer a config package from the toolchain; fallback to FetchContent if missing
find_package(pybind11 CONFIG QUIET)
if(NOT pybind11_FOUND)
  include(FetchContent)
  FetchContent_Declare(pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.11.1
  )
  FetchContent_MakeAvailable(pybind11)
endif()

# Use pybind11 helper to set correct compile/link flags
pybind11_add_module(aartzepy MODULE aartzepy.cpp)
# Link to engine when available; otherwise build a standalone stub
if (TARGET AARTZE_lib)
  target_link_libraries(aartzepy PRIVATE AARTZE_lib)
else()
  message(WARNING "AARTZE_lib target not found; building aartzepy without engine linkage (stub mode)")
endif()
target_compile_features(aartzepy PRIVATE cxx_std_17)

# Place artifacts next to other binaries (build/<config>)
set_target_properties(aartzepy PROPERTIES
  OUTPUT_NAME aartzepy
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
)

target_include_directories(aartzepy PRIVATE ${CMAKE_SOURCE_DIR})

# Post-build: copy the built extension module next to the Python editor
add_custom_command(TARGET aartzepy POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/editor_py
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:aartzepy>
          ${CMAKE_SOURCE_DIR}/editor_py/$<TARGET_FILE_NAME:aartzepy>
  COMMENT "Copying aartzepy module to editor_py/"
)

# Convenience target to run the editor from CMake
if (Python3_EXECUTABLE)
  add_custom_target(run_editor
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/editor_py/main.py
    DEPENDS aartzepy
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running AARTZE Python editor with venv interpreter"
  )
endif()
