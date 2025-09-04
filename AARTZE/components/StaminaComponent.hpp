#pragma once

/**
 * @brief Used for actions like sprinting, fighting, climbing, etc.
 */
struct StaminaComponent
{
    float currentStamina = 100.0f;
    float maxStamina = 100.0f;
    float recoveryRate = 5.0f;  // Per second
    bool isExhausted = false;
};
