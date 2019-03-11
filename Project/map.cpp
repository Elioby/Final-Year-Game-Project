#include "map.h"

#include <stdlib.h>

#include <vector>

#include "mesh.h"
#include "entity.h"
#include "asset_manager.h"
#include "debug.h"

u32 terrain_max_x = 32;
u32 terrain_max_z = 32;

// @Todo: make a map object that holds entity and cover list

std::vector<cover*> cover_list;

void map_init()
{
	u32 friendly_count = 4;
	for(u32 i = 0; i < friendly_count; i++) 
	{
		entity_add(vec3(1, 0, i * 2 + 1 + terrain_max_z / 2 - friendly_count), TEAM_FRIENDLY);
	}

	u32 enemy_count = 4;
	for(u32 i = 0; i < enemy_count; i++) 
	{
		entity_add(vec3(terrain_max_x - 2, 0, i * 2 + 1 + terrain_max_z / 2 - enemy_count), TEAM_ENEMY);
	}

	map_add_cover(vec3(5.0f, 0.0f, 5.0f));
	map_add_cover(vec3(5.0f, 0.0f, 6.0f));
	map_add_cover(vec3(5.0f, 0.0f, 7.0f));
	map_add_cover(vec3(4.0f, 0.0f, 8.0f));

	map_add_cover(vec3(10.0f, 0.0f, 4.0f));
	map_add_cover(vec3(11.0f, 0.0f, 4.0f));
	map_add_cover(vec3(12.0f, 0.0f, 4.0f));
	map_add_cover(vec3(13.0f, 0.0f, 4.0f));
	map_add_cover(vec3(14.0f, 0.0f, 4.0f));
	map_add_cover(vec3(15.0f, 0.0f, 4.0f));
	map_add_cover(vec3(16.0f, 0.0f, 4.0f));
}

mesh* map_generate_terrain_mesh()
{
	// x * y squares, 2 triangles per square, 3 vertices per triangle
	u32 vertex_count = terrain_max_x * terrain_max_z * 3 * 2;
	pos_normal_vertex* vertices = (pos_normal_vertex*) malloc(vertex_count * sizeof(pos_normal_vertex));

	u32 i = 0;
	u32 up_normal = pack_vec3_into_u32(vec3(0.0f, 1.0f, 0.0f));

	for (float x = 0; x < terrain_max_x; x++)
	{
		for (float z = 0; z < terrain_max_z; z++)
		{
			vertices[i++] = { x, 0, z, 0.0f, 0.0f, up_normal };
			vertices[i++] = { x + 1.0f, 0, z + 1.0f, 0.0f, 0.0f, up_normal };
			vertices[i++] = { x + 1.0f, 0, z, 0.0f, 0.0f, up_normal };
			vertices[i++] = { x, 0, z, 0.0f, 0.0f, up_normal };
			vertices[i++] = { x, 0, z + 1.0f, 0.0f, 0.0f, up_normal };
			vertices[i++] = { x + 1.0f, 0, z + 1.0f, 0.0f, 0.0f, up_normal };
		}
	}

	u16* indices = (u16*) malloc(vertex_count * sizeof(u16));

	assert(vertex_count < UINT16_MAX && "Vertex count must be less than max size of u16");

	for(u16 i = 0; i < vertex_count; i++)
	{
		indices[i] = i;
	}

	return mesh_create("terrain", vertices, vertex_count, indices, vertex_count);
}

void map_add_cover(vec3 pos)
{
	cover* cov = (cover*)malloc(sizeof(cover));
	cov->pos = pos;

	cover_list.push_back(cov);
}

cover* map_get_adjacent_cover(vec3 start, vec3 closest_to)
{
	float smallest_distance = 0.0f;
	cover* closest_cover = NULL;

	for(i8 x = -1; x <= 1; x++)
	{
		for(i8 y = -1; y <= 1; y++)
		{
			vec3 block_pos = start;
			block_pos.x += x;
			block_pos.y += y;

			cover* cov = map_get_cover_at_block(block_pos);
			if(cov)
			{
				float distance = map_distance_squared(start, cov->pos);
				if(!closest_cover || smallest_distance < distance)
				{
					closest_cover = cov;
					smallest_distance = distance;
				}
			}
		}
	}

	return closest_cover;
}

std::vector<vec3> map_debug_los;

bool map_has_los_internal(vec3 start, vec3 end)
{
	vec3 direction = glm::normalize(end - start);
	vec3 step = direction * MAP_RAYTRACE_ACCURACY;

	u32 timeout = 0;
	u32 max_distance = (u32)ceil(sqrt((float) terrain_max_x * terrain_max_x + (float) terrain_max_z * terrain_max_z) / MAP_RAYTRACE_ACCURACY);
	vec3 step_progress = start;
	vec3 last_block_pos = vec3(-1.0f);
	while(timeout++ < max_distance)
	{
		step_progress += step;

		vec3 next_step_block_pos = map_get_block_pos(step_progress);

		// only eval if this is a new block than last
		if(!map_pos_equal(next_step_block_pos, last_block_pos))
		{
			// if we reached the target, we have full los
			if(map_pos_equal(next_step_block_pos, end)) return true;

			// if this block is cover, they have no los
			if(map_get_cover_at_block(next_step_block_pos)) return false; 

			last_block_pos = next_step_block_pos;
		}
	}

	return false;
}

bool map_has_los(entity* ent1, entity* ent2)
{
	// how leinient is the los algorithm, in blocks
	float lean_distance = 0.5f;

	return map_has_los_internal(ent1->pos, ent2->pos) ||
		map_has_los_internal(vec3(ent1->pos.x + lean_distance, ent1->pos.y, ent1->pos.z), ent2->pos) ||
		map_has_los_internal(vec3(ent1->pos.x - lean_distance, ent1->pos.y, ent1->pos.z), ent2->pos) ||
		map_has_los_internal(vec3(ent1->pos.x, ent1->pos.y, ent1->pos.z + lean_distance), ent2->pos) ||
		map_has_los_internal(vec3(ent1->pos.x, ent1->pos.y, ent1->pos.z - lean_distance), ent2->pos) ||
		map_has_los_internal(vec3(ent2->pos.x + lean_distance, ent2->pos.y, ent2->pos.z), ent1->pos) ||
		map_has_los_internal(vec3(ent2->pos.x - lean_distance, ent2->pos.y, ent2->pos.z), ent1->pos) ||
		map_has_los_internal(vec3(ent2->pos.x, ent2->pos.y, ent2->pos.z + lean_distance), ent1->pos) ||
		map_has_los_internal(vec3(ent2->pos.x, ent2->pos.y, ent2->pos.z - lean_distance), ent1->pos);
}

float map_get_los_angle(entity* inflict_ent, entity* target_ent)
{
	// @Todo: this is a bit much, maybe we only need to check along the opposite axis of the direction
	float los_amount = 1.0f;

	cover* closest_cover = map_get_adjacent_cover(target_ent->pos, inflict_ent->pos);

	if(closest_cover)
	{
		vec3 cover_to_covered_vector = glm::normalize(closest_cover->pos - target_ent->pos);
		vec3 cover_to_shooter = glm::normalize(inflict_ent->pos - closest_cover->pos);

		float angle = glm::dot(cover_to_covered_vector, cover_to_shooter);

		// if the angle is negative, that means that the cover is behind them (they're not covered!)
		if(angle >= 0.0f)
		{
			los_amount = (1.0f - angle);
		}
	}

	return los_amount;
}

vec3 map_get_block_pos(vec3 pos)
{
	// @Volatile: there's no y axis support here D:
	return vec3(floor(pos.x + 0.5f), 0.0f, floor(pos.z + 0.5f));
}

bool map_pos_equal(vec3 pos1, vec3 pos2)
{
	// @Volatile: there's no y axis support here
	return pos1.x == pos2.x && pos1.z == pos2.z;
}

float map_distance_squared(vec3 pos1, vec3 pos2)
{
	// @Volatile: no y support
	return pow(abs(pos1.x - pos2.x), 2) + pow(abs(pos1.z - pos2.z), 2);
}

cover* map_get_cover_at_block(vec3 block_pos)
{
	// @Todo: it's very slow to loop through for lookup :(
	for(u32 i = 0; i < cover_list.size(); i++)
	{
		cover* cov = cover_list[i];

		if(map_pos_equal(cov->pos, block_pos))
		{
			return cov;
		}
	}

	return NULL;
}

entity* map_get_entity_at_block(vec3 block_pos)
{
	// @Todo: it's very slow to loop through for lookup :(
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* ent = entities[i];

		if(map_pos_equal(ent->pos, block_pos))
		{
			return ent;
		}
	}

	return NULL;
}

void map_draw()
{

}