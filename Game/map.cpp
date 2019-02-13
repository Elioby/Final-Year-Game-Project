#include "map.h"

#include <stdlib.h>

#include <vector>

#include "mesh.h"
#include "entity.h"

u32 terrain_max_x = 32;
u32 terrain_max_z = 32;

// @Todo: make a map object that holds entity and cover list

std::vector<cover*> cover_list;

void add_cover(vec3 pos)
{
	cover* cov = (cover*) malloc(sizeof(cover));
	cov->pos = pos;

	cover_list.push_back(cov);
}

void map_init()
{
	u32 friendly_count = 6;
	for(u32 i = 0; i < friendly_count; i++) {
		entity_add(vec3(1, 0, i * 2 + 1 + terrain_max_z / 2 - friendly_count), false);
	}

	u32 enemy_count = 6;
	for(u32 i = 0; i < enemy_count; i++) {
		entity_add(vec3(terrain_max_x - 2, 0, i * 2 + 1 + terrain_max_z / 2 - enemy_count), true);
	}

	add_cover(vec3(5.0f, 0.0f, 5.0f));
	add_cover(vec3(5.0f, 0.0f, 6.0f));
	add_cover(vec3(5.0f, 0.0f, 7.0f));
}

mesh load_terrain_mesh()
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

	return create_mesh(vertices, vertex_count);
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