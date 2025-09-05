add_library(aartze_physics STATIC Physics.cpp)
target_include_directories(aartze_physics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_compile_features(aartze_physics PUBLIC cxx_std_17)

# Bullet linking via config targets when available, otherwise fallback to module variables
if (TARGET Bullet::BulletDynamics)
  target_link_libraries(aartze_physics PUBLIC aartze_core Bullet::BulletDynamics Bullet::BulletCollision Bullet::LinearMath)
elseif (TARGET BulletDynamics)
  target_link_libraries(aartze_physics PUBLIC aartze_core BulletDynamics BulletCollision LinearMath)
elseif (TARGET Bullet3Dynamics)
  # vcpkg sometimes exports non-namespaced targets
  target_link_libraries(aartze_physics PUBLIC aartze_core Bullet3Dynamics BulletCollision LinearMath)
elseif (BULLET_LIBRARIES)
  target_include_directories(aartze_physics PUBLIC ${BULLET_INCLUDE_DIRS})
  target_link_libraries(aartze_physics PUBLIC aartze_core ${BULLET_LIBRARIES})
else()
  message(FATAL_ERROR "Bullet not found: expected targets Bullet::BulletDynamics or module variables BULLET_LIBRARIES")
endif()
