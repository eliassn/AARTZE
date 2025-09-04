#pragma once

/**
 * @brief Component for controlling driving behavior of vehicles or players in vehicles.
 */
struct DrivingComponent
{
    bool isDriving = false;
    float acceleration = 0.0f;
    float steeringAngle = 0.0f;
    bool handbrake = false;
    bool brake = false;
};
