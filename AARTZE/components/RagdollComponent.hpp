#pragma once

/**
 * @brief Enables ragdoll physics on entity death or knockdown.
 */
struct RagdollComponent
{
    bool isActive = false;
    float fallForce = 0.0f;
};
