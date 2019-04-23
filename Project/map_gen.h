#pragma once

#include "general.h"
#include "mesh.h"

struct map_road_segment
{
	vec2 pos;
	vec2 scale;
};

// the segments of the map, split up by the roads
struct map_segment
{
	vec2 pos;
	vec2 scale;
};

mesh* map_gen_terrain_mesh();

void map_gen_roads();
void map_gen();

// @Todo: remove
void split_segments_on_point(u32 split_point, bool split_on_x);
