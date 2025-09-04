#pragma once
#include <imgui.h>

bool EntityHasTransform(int entity);
void EditTransform(int entity);

bool EntityHasMaterial(int entity);
void EditMaterial(int entity);

// Optional: add NavAgent to selected entity
bool EntityHasNavAgent(int entity);
void AddNavAgent(int entity);
