#pragma once

/**
 * @brief Tracks aiming state for entities (e.g. for ADS, crosshair, etc.).
 */
struct AimingComponent
{
    bool isAiming = false;
    float aimSensitivity = 0.75f;
    float aimZoomLevel = 1.0f;
};
