cmake_minimum_required(VERSION 3.16)
project(AARTZE)

# Suppress FetchContent_Populate(name-only) deprecation for repos without CMakeLists (e.g., imgui)
if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()
option(BUILD_ENGINE "Build engine library and demo" ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(thirdparty/glm-master)

# Ensure vcpkg toolchain is picked up when presets fail to pass it on first configure
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(_VCPKG_TOOLCHAIN "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
  if(EXISTS "${_VCPKG_TOOLCHAIN}")
    set(CMAKE_TOOLCHAIN_FILE "${_VCPKG_TOOLCHAIN}" CACHE STRING "" FORCE)
    include("${_VCPKG_TOOLCHAIN}")
  endif()
endif()

include(FetchContent)

# Robust vcpkg discovery for configure-from-preset issues
# If the toolchain wasn't applied yet, try the local vcpkg_installed tree so
# find_package(assimp CONFIG) can succeed during configure.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  # Try to locate assimp config inside common local vcpkg layout
  foreach(_base "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}")
    foreach(_trip "x64-windows" "x86-windows")
      set(_cand "${_base}/vcpkg_installed/${_trip}/share/assimp/assimp-config.cmake")
      if(EXISTS "${_cand}")
        get_filename_component(_assimp_dir "${_cand}" DIRECTORY)
        set(assimp_DIR "${_assimp_dir}" CACHE PATH "")
      endif()
    endforeach()
  endforeach()
endif()

# ===== JSON (Header Only) =====
# Prefer vcpkg/system package; fall back to FetchContent only if missing.
find_package(nlohmann_json CONFIG QUIET)
if(NOT nlohmann_json_FOUND)
  message(STATUS "nlohmann_json not found via package config; falling back to FetchContent")
  FetchContent_Declare(
      json
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG v3.11.2
      GIT_SHALLOW TRUE
  )
  FetchContent_GetProperties(json)
  if(NOT json_POPULATED)
      FetchContent_Populate(json)
  endif()
  add_library(nlohmann_json INTERFACE)
  target_include_directories(nlohmann_json INTERFACE ${json_SOURCE_DIR}/include)
endif()

find_package(assimp CONFIG REQUIRED)

# ImGui disabled

# ===== Engine Sources =====
if(BUILD_ENGINE)
    find_package(OpenGL REQUIRED)
    find_package(Threads REQUIRED)
    find_package(OpenAL CONFIG QUIET)
    # Try Bullet via config first, then fall back to module mode
    find_package(Bullet CONFIG QUIET)
    if(NOT TARGET Bullet::BulletDynamics)
        find_package(Bullet REQUIRED)
    endif()
    # Optional/Pluggable dependencies for richer editor/engine integration
    find_package(EnTT CONFIG QUIET)
    find_package(gainput CONFIG QUIET)
    find_package(cgltf CONFIG QUIET)
    find_package(ozz-animation CONFIG QUIET)
    # Jolt physics (optional) with Bullet fallback
    find_package(Jolt CONFIG QUIET)
    # Lua + sol2 are optional; enable when found
    find_package(Lua QUIET)
    find_package(unofficial-lua CONFIG QUIET)
    find_package(sol2 CONFIG QUIET)
    find_package(glm CONFIG QUIET)

    # ===== GLAD via vcpkg =====
    find_package(glad CONFIG REQUIRED)

    # ===== GLFW =====
    # Use vcpkg-provided glfw3 to avoid network fetch during configure
    find_package(glfw3 CONFIG REQUIRED)
    file(GLOB_RECURSE ENGINE_SOURCES
        ${CMAKE_SOURCE_DIR}/AARTZE/**/*.cpp
        ${CMAKE_SOURCE_DIR}/AARTZE/systems/**/*.cpp
    )
    # Exclude editor sources from the core engine by default.
    # Use only forward slashes in regex as CMake normalizes paths.
    list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/AARTZE/editor/.*")
    # Exclude experimental ui2 path which is incomplete
    list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/AARTZE/ui2/.*")

    # Re-include non-UI editor utilities needed by runtime
    list(APPEND ENGINE_SOURCES
        ${CMAKE_SOURCE_DIR}/AARTZE/editor/EditorActions.cpp
        ${CMAKE_SOURCE_DIR}/AARTZE/editor/EditorState.cpp)
    list(APPEND ENGINE_SOURCES
        ${CMAKE_SOURCE_DIR}/AARTZE/utils/ScriptLoader.cpp
        ${CMAKE_SOURCE_DIR}/AARTZE/thirdparty/stb_impl.cpp)

    # Optional legacy ImGui editor UI (off by default)
    option(BUILD_LEGACY_IMGUI "Build legacy ImGui UIManager" OFF)
    if(BUILD_LEGACY_IMGUI)
    endif()

option(USE_VULKAN_RT "Enable Vulkan ray tracing backend" OFF)
if(USE_VULKAN_RT)
    find_package(Vulkan QUIET)
    if(Vulkan_FOUND)
        message(STATUS "Vulkan found - building ray tracing backend")
        set(HAVE_VULKAN_RT TRUE)
    else()
        message(WARNING "Vulkan not found - ray tracing disabled")
        set(HAVE_VULKAN_RT FALSE)
    endif()
endif()

    add_library(AARTZE_lib STATIC ${ENGINE_SOURCES})
    target_compile_definitions(AARTZE_lib PUBLIC GLM_ENABLE_EXPERIMENTAL)
    if(HAVE_VULKAN_RT)
        target_compile_definitions(AARTZE_lib PUBLIC USE_VULKAN_RT)
    endif()

    target_include_directories(AARTZE_lib PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/AARTZE
    ${CMAKE_SOURCE_DIR}/AARTZE/core
    ${CMAKE_SOURCE_DIR}/AARTZE/utils
    ${CMAKE_SOURCE_DIR}/AARTZE/components
    ${CMAKE_SOURCE_DIR}/NAFEngine/components
    ${CMAKE_SOURCE_DIR}/AARTZE/systems
        ${CMAKE_SOURCE_DIR}/AARTZE/World

    thirdparty/glm-master
    ${glfw_SOURCE_DIR}/include
    ${ASSIMP_INCLUDE_DIRS}
)

    target_link_libraries(AARTZE_lib PRIVATE
    nlohmann_json
    glad::glad
    glfw
    OpenGL::GL
    assimp::assimp
    Threads::Threads
    $<$<BOOL:${USE_VULKAN_RT}>:Vulkan::Vulkan>
    $<$<BOOL:${OpenAL_FOUND}>:OpenAL::OpenAL>
)
    # Optional targets if present
    if(TARGET EnTT::EnTT)
        target_link_libraries(AARTZE_lib PUBLIC EnTT::EnTT)
    endif()
    if(TARGET gainput::gainput)
        target_link_libraries(AARTZE_lib PRIVATE gainput::gainput)
        target_compile_definitions(AARTZE_lib PRIVATE HAVE_GAINPUT)
    elseif(TARGET unofficial::gainput::gainput)
        target_link_libraries(AARTZE_lib PRIVATE unofficial::gainput::gainput)
        target_compile_definitions(AARTZE_lib PRIVATE HAVE_GAINPUT)
    endif()
    if(TARGET cgltf::cgltf)
        target_link_libraries(AARTZE_lib PRIVATE cgltf::cgltf)
        target_compile_definitions(AARTZE_lib PRIVATE HAVE_CGLTF)
    elseif(TARGET cgltf)
        target_link_libraries(AARTZE_lib PRIVATE cgltf)
        target_compile_definitions(AARTZE_lib PRIVATE HAVE_CGLTF)
    else()
        find_path(CGLTF_INCLUDE_DIRS "cgltf.h")
        if(CGLTF_INCLUDE_DIRS)
            target_include_directories(AARTZE_lib PRIVATE ${CGLTF_INCLUDE_DIRS})
            target_compile_definitions(AARTZE_lib PRIVATE HAVE_CGLTF)
        endif()
    endif()
    if(TARGET ozz_animation::ozz_animation)
        target_link_libraries(AARTZE_lib PRIVATE ozz_animation::ozz_animation)
        target_compile_definitions(AARTZE_lib PRIVATE HAVE_OZZ_ANIMATION)
    endif()
    if(TARGET Jolt::Jolt)
        target_link_libraries(AARTZE_lib PRIVATE Jolt::Jolt)
        target_compile_definitions(AARTZE_lib PRIVATE HAVE_JOLT_PHYSICS)
    elseif(TARGET Jolt)
        target_link_libraries(AARTZE_lib PRIVATE Jolt)
        target_compile_definitions(AARTZE_lib PRIVATE HAVE_JOLT_PHYSICS)
    endif()
    if(WIN32)
        target_link_libraries(AARTZE_lib PRIVATE comdlg32 ole32)
    endif()
    # Bullet linking (config targets or module variables)
    if(TARGET Bullet::BulletDynamics)
        target_link_libraries(AARTZE_lib PRIVATE Bullet::BulletDynamics Bullet::BulletCollision Bullet::LinearMath)
    elseif(BULLET_LIBRARIES)
        target_include_directories(AARTZE_lib PUBLIC ${BULLET_INCLUDE_DIRS})
        target_link_libraries(AARTZE_lib PRIVATE ${BULLET_LIBRARIES})
    else()
        message(FATAL_ERROR "Bullet not found: install bullet3 (vcpkg) or provide Bullet package")
    endif()

    # Lua optional linking and defines
    set(HAVE_LUA OFF)
    if(TARGET Lua::Lua)
        target_link_libraries(AARTZE_lib PRIVATE Lua::Lua)
        set(HAVE_LUA ON)
    elseif(TARGET unofficial-lua::lua)
        target_link_libraries(AARTZE_lib PRIVATE unofficial-lua::lua)
        set(HAVE_LUA ON)
    elseif(LUA_LIBRARIES)
        target_include_directories(AARTZE_lib PUBLIC ${LUA_INCLUDE_DIR})
        target_link_libraries(AARTZE_lib PRIVATE ${LUA_LIBRARIES})
        set(HAVE_LUA ON)
    endif()
    if(TARGET sol2)
        target_link_libraries(AARTZE_lib PRIVATE sol2)
    endif()
    if(HAVE_LUA)
        target_compile_definitions(AARTZE_lib PRIVATE HAVE_LUA)
    else()
        message(STATUS "Lua not found; scripting will be disabled.")
    endif()

    # GLM link if available
    if(TARGET glm::glm)
        target_link_libraries(AARTZE_lib PRIVATE glm::glm)
    endif()
    target_compile_definitions(AARTZE_lib PRIVATE $<$<BOOL:${OpenAL_FOUND}>:HAVE_OPENAL>)

    # Toggle experimental Slate-like UI2 (OFF by default due to WIP headers)
    option(USE_UI2 "Enable UI2 (experimental)" OFF)
    if(USE_UI2)
        target_compile_definitions(AARTZE_lib PUBLIC USE_UI2)
    endif()

    # Optional ImGui-based Editor UI (off by default).
    # Enable with: -DBUILD_AARTZE_IMGUI_EDITOR=ON
    option(BUILD_AARTZE_IMGUI_EDITOR "Build AARTZE ImGui editor UI" OFF)
    if(BUILD_AARTZE_IMGUI_EDITOR)
        # Use a local Dear ImGui checkout instead of fetching from network.
        # Set IMGUI_DIR to point to your imgui folder (default: thirdparty/imgui).
        set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/thirdparty/imgui CACHE PATH "Path to local Dear ImGui sources")
        if(NOT EXISTS ${IMGUI_DIR}/imgui.h)
            message(FATAL_ERROR "Dear ImGui not found at IMGUI_DIR='${IMGUI_DIR}'. Set -DIMGUI_DIR=path/to/imgui or place imgui sources under thirdparty/imgui.")
        endif()

        add_library(imgui STATIC
            ${IMGUI_DIR}/imgui.cpp
            ${IMGUI_DIR}/imgui_draw.cpp
            ${IMGUI_DIR}/imgui_tables.cpp
            ${IMGUI_DIR}/imgui_widgets.cpp
            ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
            ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
            ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
        )
        target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
        target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
        target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

        # ImGuizmo and imnodes (prefer local checkouts; fallback to FetchContent)
        set(IMGUIZMO_DIR "" CACHE PATH "Path to local ImGuizmo (optional)")
        set(IMNODES_DIR  "" CACHE PATH "Path to local imnodes (optional)")

        if(IMGUIZMO_DIR AND EXISTS ${IMGUIZMO_DIR}/ImGuizmo.h)
          set(imguizmo_SOURCE_DIR ${IMGUIZMO_DIR})
        else()
          include(FetchContent)
          FetchContent_Declare(
            imguizmo
            GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
            GIT_TAG master
            GIT_SHALLOW TRUE
          )
          FetchContent_MakeAvailable(imguizmo)
        endif()
        add_library(ImGuizmo STATIC ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp)
        target_include_directories(ImGuizmo PUBLIC ${imguizmo_SOURCE_DIR} ${IMGUI_DIR})
        target_link_libraries(ImGuizmo PUBLIC imgui)

        if(IMNODES_DIR AND EXISTS ${IMNODES_DIR}/imnodes.h)
          set(imnodes_SOURCE_DIR ${IMNODES_DIR})
        else()
          include(FetchContent)
          FetchContent_Declare(
            imnodes
            GIT_REPOSITORY https://github.com/Nelarius/imnodes.git
            GIT_TAG v0.5
            GIT_SHALLOW TRUE
          )
          FetchContent_MakeAvailable(imnodes)
        endif()
        add_library(imnodes_lib STATIC ${imnodes_SOURCE_DIR}/imnodes.cpp)
        target_include_directories(imnodes_lib PUBLIC ${imnodes_SOURCE_DIR})
        target_link_libraries(imnodes_lib PUBLIC imgui)
        # ImNodes requires IMGUI_DEFINE_MATH_OPERATORS to be defined before including imgui.h
        target_compile_definitions(imnodes_lib PRIVATE IMGUI_DEFINE_MATH_OPERATORS)
        # Silence redefinition warning on MSVC when both CLI and header define it
        if(MSVC)
          target_compile_options(imnodes_lib PRIVATE /wd4005)
        endif()

        # Make ImGui headers available to engine sources that embed UI hooks
        target_include_directories(AARTZE_lib PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
        target_compile_definitions(AARTZE_lib PUBLIC AARTZE_WITH_IMGUI=1)

        # AARTZE ImGui editor library
        add_library(AARTZE_imgui_editor STATIC
            ${CMAKE_SOURCE_DIR}/AARTZE/editor/AartzeEditor.cpp
            ${CMAKE_SOURCE_DIR}/AARTZE/editor/Renderer.cpp
        )
        target_include_directories(AARTZE_imgui_editor PUBLIC 
            ${CMAKE_SOURCE_DIR}/AARTZE/editor ${imguizmo_SOURCE_DIR} ${imnodes_SOURCE_DIR})
        target_link_libraries(AARTZE_imgui_editor PUBLIC imgui AARTZE_lib ImGuizmo imnodes_lib)
        target_compile_definitions(AARTZE_imgui_editor PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

        # M3: Importer + Scene + GPU Upload (editor/runtime sources)
        target_sources(AARTZE_imgui_editor PRIVATE
          ${CMAKE_SOURCE_DIR}/AARTZE/runtime/Mesh.cpp
          ${CMAKE_SOURCE_DIR}/AARTZE/runtime/GLMesh.cpp
          ${CMAKE_SOURCE_DIR}/AARTZE/runtime/Scene.cpp
          ${CMAKE_SOURCE_DIR}/AARTZE/runtime/Importer.cpp
        )
        target_include_directories(AARTZE_imgui_editor PUBLIC
          ${CMAKE_SOURCE_DIR}/AARTZE/runtime
        )
        target_link_libraries(AARTZE_imgui_editor PUBLIC assimp::assimp)

        # Ensure engine sources can include ImGuizmo headers when AARTZE_WITH_IMGUI is enabled
        target_include_directories(AARTZE_lib PUBLIC ${imguizmo_SOURCE_DIR} ${imnodes_SOURCE_DIR})

        # Standalone editor demo executable (keeps game exe untouched)
        add_executable(AARTZE_ImGuiDemo ${CMAKE_SOURCE_DIR}/AARTZE/editor/app_main.cpp)
        target_link_libraries(AARTZE_ImGuiDemo PRIVATE 
            AARTZE_imgui_editor AARTZE_lib imgui ImGuizmo imnodes_lib glfw glad::glad OpenGL::GL)
        target_compile_definitions(AARTZE_ImGuiDemo PRIVATE AARTZE_WITH_IMGUI=1 IMGUI_IMPL_OPENGL_LOADER_GLAD)
    endif()

# ===== Game Executable =====
    add_executable(AARTZE main.cpp)
    target_link_libraries(AARTZE PRIVATE AARTZE_lib )
    if(BUILD_AARTZE_IMGUI_EDITOR)
        target_link_libraries(AARTZE PRIVATE imgui AARTZE_imgui_editor)
        target_compile_definitions(AARTZE PRIVATE AARTZE_WITH_IMGUI=1)
    endif()

    # Copy runtime resources next to the executable (if present)
    if(EXISTS ${CMAKE_SOURCE_DIR}/assets)
        file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
    else()
        message(STATUS "Assets directory not found; skipping copy")
    endif()
    if(EXISTS ${CMAKE_SOURCE_DIR}/shaders)
        file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
    else()
        message(STATUS "Shaders directory not found; skipping copy")
    endif()
    if(EXISTS ${CMAKE_SOURCE_DIR}/triangle.b64)
        file(COPY triangle.b64 DESTINATION ${CMAKE_BINARY_DIR})
    else()
        message(STATUS "triangle.b64 not found; skipping copy")
    endif()
endif()

# ----- AARTZE modular build (opt-in) -----
option(AARTZE_ENABLE_MODULAR "Enable new modular AARTZE build (libraries + sandbox)" ON)
if (AARTZE_ENABLE_MODULAR)
  add_subdirectory(src/aartze)
endif()

# Python bindings (pybind11) — adds aartzepy module linking AARTZE_lib
# Optional: Python bindings (pybind11) — OFF by default so C++/ImGui builds never break
option(BUILD_AARTZE_PYTHON "Build Python editor bindings (pybind11)" OFF)
if (BUILD_AARTZE_PYTHON)
  # Prefer binding against the local venv's Python if present
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
  find_package(pybind11 CONFIG QUIET)
  if (pybind11_FOUND AND EXISTS ${CMAKE_SOURCE_DIR}/python_bindings/CMakeLists.txt)
    add_subdirectory(python_bindings)
  else()
    message(WARNING "pybind11 not found; skipping python_bindings. Install via vcpkg or set CMAKE_PREFIX_PATH.")
  endif()
endif()

# Enable tests only if a tests directory is present
if(EXISTS ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt OR IS_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Tests directory not found; skipping tests")
endif()





