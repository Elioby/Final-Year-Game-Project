#include "map.h"

#include "mesh.h"
#include "entity.h"

u32 terrain_max_x = 25;
u32 terrain_max_z = 25;

void map_init()
{
	add_default_entity(vec3(0.0f, 0.0f, 0.0f));
	add_default_entity(vec3(10.0f, 0.0f, 0.0f));
	add_default_entity(vec3(5.0f, 0.0f, 10.0f));
	add_default_entity(vec3(20.0f, 0.0f, 5.0f));
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