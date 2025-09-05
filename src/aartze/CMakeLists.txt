cmake_minimum_required(VERSION 3.16)

project(aartze_modules)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Consolidated deps (expected to be provided by vcpkg)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)
find_package(ozz-animation CONFIG QUIET)
find_package(pybind11 CONFIG REQUIRED)
find_package(OpenAL CONFIG QUIET)

option(AARTZE_WITH_VULKAN "Enable Vulkan headers for RHI" OFF)
if(AARTZE_WITH_VULKAN)
  find_package(VulkanHeaders CONFIG REQUIRED)
endif()

add_subdirectory(core)
add_subdirectory(math)
add_subdirectory(ecs)
add_subdirectory(geometry)
if (TARGET ozz_animation::ozz_animation)
  add_subdirectory(animation)
else()
  message(STATUS "ozz-animation not found; skipping aartze_animation module")
endif()
add_subdirectory(render)
add_subdirectory(import)
add_subdirectory(physics)
add_subdirectory(scripting)

add_library(aartze STATIC)
target_link_libraries(aartze PUBLIC aartze_core aartze_math aartze_ecs aartze_geometry aartze_render aartze_import aartze_physics aartze_scripting)
if (TARGET aartze_animation)
  target_link_libraries(aartze PUBLIC aartze_animation)
endif()
target_include_directories(aartze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(aartze_sandbox ../apps/sandbox/main.cpp)
target_link_libraries(aartze_sandbox PRIVATE aartze glfw glad::glad)
if (WIN32)
  target_link_libraries(aartze_sandbox PRIVATE OpenGL32)
endif()

# Editor runner (optional)
option(AARTZE_BUILD_EDITOR "Enable Python editor run target" ON)
if (AARTZE_BUILD_EDITOR)
  add_custom_target(aartze_editor_run
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=$<TARGET_FILE_DIR:aartze_py>"
            ${CMAKE_COMMAND} -E echo "Launch with: python -m aartze_editor"
    COMMENT "Run the Python editor with PYTHONPATH set to the pybind11 module"
  )
endif()
