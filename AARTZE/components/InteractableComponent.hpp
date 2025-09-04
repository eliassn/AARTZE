#pragma once

/**
 * @brief Marks objects (doors, levers, NPCs) as interactable.
 */
struct InteractableComponent
{
    bool canInteract = true;
    float interactionRadius = 2.5f;
    bool requiresLineOfSight = true;
};
