#pragma once

/**
 * @brief Input flags for controlling player movement and actions.
 */
struct InputComponent
{
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool jump = false;
    bool fire = false;
    bool interact = false;
    bool run = false;
    bool sneak = false;
    bool aim = false;
};
