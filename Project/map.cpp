#include "map.h"

#include <stdlib.h>

#include <vector>

#include "mesh.h"
#include "entity.h"

u32 terrain_max_x = 32;
u32 terrain_max_z = 32;

// @Todo: make a map object that holds entity and cover list

std::vector<cover*> cover_list;

void map_init()
{
	u32 friendly_count = 6;
	for(u32 i = 0; i < friendly_count; i++) 
	{
		entity_add(vec3(1, 0, i * 2 + 1 + terrain_max_z / 2 - friendly_count), false);
	}

	u32 enemy_count = 6;
	for(u32 i = 0; i < enemy_count; i++) 
	{
		entity_add(vec3(terrain_max_x - 2, 0, i * 2 + 1 + terrain_max_z / 2 - enemy_count), true);
	}

	map_add_cover(vec3(5.0f, 0.0f, 5.0f));
	map_add_cover(vec3(5.0f, 0.0f, 6.0f));
	map_add_cover(vec3(5.0f, 0.0f, 7.0f));
}

mesh map_generate_terrain_mesh()
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

	return mesh_create(vertices, vertex_count);
}

void map_add_cover(vec3 pos)
{
	cover* cov = (cover*)malloc(sizeof(cover));
	cov->pos = pos;

	cover_list.push_back(cov);
}

bool map_check_los(vec3 start, vec3 end)
{
	// the step of the world raytrace
	float accuracy = 0.25f;
	vec3 direction = glm::normalize(end - start);
	vec3 step = direction * accuracy;

	u32 timeout = 0;
	u32 max_distance = (u32)ceil(sqrt((float)terrain_max_x * terrain_max_x + (float)terrain_max_z * terrain_max_z) / accuracy);
	vec3 step_progress = start;
	vec3 last_block_pos = vec3(-1.0f);
	while (timeout++ < max_distance)
	{
		step_progress += step;

		vec3 next_step_block_pos = map_get_block_pos(step_progress);

		// only eval if this is a new block than last
		if (!map_pos_equal(next_step_block_pos, last_block_pos))
		{
			// if this block is cover, we do not have los
			if (map_get_cover_at_block(next_step_block_pos)) return false;

			// if we reached the target, we have los
			if (map_pos_equal(next_step_block_pos, end)) return true;

			last_block_pos = next_step_block_pos;
		}
	}

	return false;
}

vec3 map_get_block_pos(vec3 pos)
{
	// @Volatile: there's no y axis support here D:
	return vec3(floor(pos.x + 0.5f), 0.0f, floor(pos.z + 0.5f));
}

bool map_pos_equal(vec3 pos1, vec3 pos2)
{
	glm::vec<3, bool> equality = glm::equal(pos1, pos2);

	return equality.x && equality.z;
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

void map_draw()
{

}