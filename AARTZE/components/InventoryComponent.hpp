#pragma once
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Component holding all items an entity can carry.
 */
struct InventoryComponent
{
    std::vector<uint32_t> itemIds;
    uint32_t maxCapacity = 30;
};
