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
include(FetchContent)

# ===== JSON (Header Only) =====
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
endif()
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${json_SOURCE_DIR}/include)

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
    # Lua + sol2 are optional; enable when found
    find_package(Lua QUIET)
    find_package(unofficial-lua CONFIG QUIET)
    find_package(sol2 CONFIG QUIET)
    find_package(glm CONFIG QUIET)

    # ===== GLAD via vcpkg =====
    find_package(glad CONFIG REQUIRED)

    # ===== GLFW =====
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
    FetchContent_MakeAvailable(glfw)
    file(GLOB_RECURSE ENGINE_SOURCES
        ${CMAKE_SOURCE_DIR}/AARTZE/**/*.cpp
        ${CMAKE_SOURCE_DIR}/AARTZE/systems/**/*.cpp
    )
    list(FILTER ENGINE_SOURCES EXCLUDE REGEX "(/|\\)AARTZE(/|\\)editor(/|\\)")
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

# ===== Game Executable =====
    add_executable(AARTZE main.cpp)
    target_link_libraries(AARTZE PRIVATE AARTZE_lib )

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

# Enable tests only if a tests directory is present
if(EXISTS ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt OR IS_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Tests directory not found; skipping tests")
endif()





