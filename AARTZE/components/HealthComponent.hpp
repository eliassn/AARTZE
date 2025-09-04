#pragma once

/**
 * @brief Tracks the health of any entity (NPC, player, vehicle, animal, etc).
 */
struct HealthComponent
{
    float currentHealth = 100.0f;
    float maxHealth = 100.0f;
    bool isInvincible = false;
};
