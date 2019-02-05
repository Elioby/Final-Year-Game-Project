#pragma once

#include "general.h"
#include "mesh.h"

extern u32 terrain_max_x;
extern u32 terrain_max_z;

void map_init();
mesh load_terrain_mesh();