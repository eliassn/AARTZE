#pragma once

/**
 * @brief Tracks law enforcement alert level.
 */
struct WantedLevelComponent
{
    int currentLevel = 0;  // 0 to 5 stars system
    float cooldownTime = 0.0f;
};
