#pragma once

#include <string>
#include <vector>

/**
 * @brief Holds temporary buffs/debuffs like bleeding, poisoned, etc.
 */
struct StatusEffectComponent
{
    std::vector<std::string> activeEffects;
};
