#pragma once
#include <cstdint>
#include <string>

/**
 * @brief Used to define a weapon held or equipped by an entity.
 */
struct WeaponComponent
{
    uint32_t weaponId = 0;
    int currentAmmo = 0;
    int maxAmmo = 30;
    bool isEquipped = true;
    float reloadTime = 2.0f;
};
