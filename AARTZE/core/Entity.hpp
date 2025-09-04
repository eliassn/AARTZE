#pragma once

#include <cstdint>

/**
 * @brief Defines the type used for entities and components.
 */
using Entity = std::uint32_t;
constexpr Entity MAX_ENTITIES = 5000;

using ComponentType = std::uint8_t;
// Maximum number of component types supported by the engine.
// Stored separately from ComponentType's underlying type so that
// the value does not wrap when the limit exceeds 255.
constexpr std::size_t MAX_COMPONENTS = 256;
