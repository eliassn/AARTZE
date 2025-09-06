add_library(aartze_input STATIC InputSystem.cpp)
target_include_directories(aartze_input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_SOURCE_DIR}/src)
target_compile_features(aartze_input PUBLIC cxx_std_17)

# Optional gainput integration from vcpkg
find_package(gainput CONFIG QUIET)
if (TARGET gainput::gainput)
  target_link_libraries(aartze_input PUBLIC gainput::gainput)
  target_compile_definitions(aartze_input PUBLIC HAVE_GAINPUT)
elseif (TARGET unofficial::gainput::gainput)
  target_link_libraries(aartze_input PUBLIC unofficial::gainput::gainput)
  target_compile_definitions(aartze_input PUBLIC HAVE_GAINPUT)
endif()

