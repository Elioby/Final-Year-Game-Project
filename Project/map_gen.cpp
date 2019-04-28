#include "map_gen.h"

#include "map.h"

// @Todo: i think this needs to be moved...
mesh* map_gen_terrain_mesh()
{
	// x * y squares, 2 triangles per square, 3 vertices per triangle
	u32 vertex_count = map_max_x * map_max_z * 3 * 2;
	pos_normal_vertex* vertices = (pos_normal_vertex*)debug_malloc(vertex_count * sizeof(pos_normal_vertex));

	u32 i = 0;
	u32 up_normal = pack_vec3_into_u32(vec3(0.0f, 1.0f, 0.0f));

	for(float x = 0; x < map_max_x; x++)
	{
		for(float z = 0; z < map_max_z; z++)
		{
			vertices[i++] = { x, 0, z, 0.0f, 0.0f, up_normal };
			vertices[i++] = { x + 1.0f, 0, z + 1.0f, 0.0f, 0.0f, up_normal };
			vertices[i++] = { x + 1.0f, 0, z, 0.0f, 0.0f, up_normal };
			vertices[i++] = { x, 0, z, 0.0f, 0.0f, up_normal };
			vertices[i++] = { x, 0, z + 1.0f, 0.0f, 0.0f, up_normal };
			vertices[i++] = { x + 1.0f, 0, z + 1.0f, 0.0f, 0.0f, up_normal };
		}
	}

	u32* indices = (u32*) debug_malloc(vertex_count * sizeof(u32));

	debug_assert(vertex_count < UINT32_MAX, "Indicies cannot exceed max size of u32");

	for(u32 i = 0; i < vertex_count; i++)
	{
		indices[i] = i;
	}

	return mesh_create("terrain", vertices, vertex_count, indices, vertex_count);
}

#define MAP_GEN_ROAD_WIDTH 5
#define MAP_GEN_MIN_ROAD_LENGTH 14

void map_gen_roads();

void map_gen_segments();
void map_gen_simplify_segments();
void split_segments_on_point(u32 split_point, bool split_on_x);

void map_gen_biomes();
void map_gen_building(map_segment seg);

// @Cleanup: this is messy and kinda a hack :(
u32 building_coverage = 0;
u32 building_count = 0;
u32 regen_tries = 0;

void map_gen()
{
	// make sure we have a clean slate
	building_coverage = 0;
	building_count = 0;

	dynarray_clear(map_road_segments);
	dynarray_clear(map_segments);
	map_clear_cover();

	// first we build the roads, and then the rest of the generation revolves around this
	map_gen_roads();

	map_gen_segments();
	map_gen_simplify_segments();

	map_gen_biomes();

	// regen if we have a bad building coverage
	if ((building_coverage / (double) (map_max_x * map_max_z) < 0.3f || building_count < 3) && regen_tries <= 30)
	{
		regen_tries++;
		map_gen();
	}
	else
	{
		printf("Retried %i times, current coverage: %f\n", regen_tries, building_coverage / (double) (map_max_x * map_max_z));
		regen_tries = 0;
	}
}

// @Todo: pass in map object?
void map_gen_roads()
{
	vec3 start = vec3(0.0f, 0.0f, map_max_z - 8.0f);
	vec3 progress = start;

	vec3 direction = vec3(1.0f, 0.0f, 0.0f);

	vec3 current_segment_start = start;
	u32 current_segment_length = -1;
	
	while(true)
	{
		bool end_of_road = progress.x >= map_max_x || progress.z >= map_max_z || progress.x < 0 || progress.z < 0;

		if(current_segment_length++ > MAP_GEN_MIN_ROAD_LENGTH || end_of_road)
		{
			double random = ((double) rand() / (double) RAND_MAX);

			// randomly change direction, but only if you're not too close to the edge of the map
			bool change_direction = random <= 0.075f;

			// check if we're close to the end (cast to s32 just incase MAP_GEN_MIN_ROAD_LENGTH - MAP_GEN_ROAD_WIDTH - 1 > map_max_x (and then we would wrap back to 2.4bil)
			bool close_to_end = progress.x >= (s32) map_max_x - MAP_GEN_MIN_ROAD_LENGTH - MAP_GEN_ROAD_WIDTH - 1 || progress.z <= MAP_GEN_MIN_ROAD_LENGTH + MAP_GEN_ROAD_WIDTH + 1;

			// also force us to end the road at 0 on the z axis
			if(close_to_end)
			{
				change_direction = direction.z == 0.0f;
			}

			if(change_direction || end_of_road)
			{
				map_road_segment seg = {};
				seg.pos = vec2(current_segment_start.x, current_segment_start.z);

				if(direction.x == 0.0f)
				{
					direction.x = 1.0f;
					direction.z = 0.0f;
					
					seg.scale = vec2(MAP_GEN_ROAD_WIDTH, current_segment_length);
					seg.pos.y -= current_segment_length - 1;
				}
				else
				{
					direction.x = 0.0f;
					direction.z = -1.0f;

					seg.scale = vec2(current_segment_length, MAP_GEN_ROAD_WIDTH);
					seg.pos.y -= MAP_GEN_ROAD_WIDTH - 1;
				}

				dynarray_add(map_road_segments, &seg);

				current_segment_length = 0;
				current_segment_start = progress;

				if(end_of_road) break;
			}
		}
		
		progress += direction;
	}
}

void map_gen_segments()
{
	map_segment starting_segment = {};
	starting_segment.pos = vec2(0.0f, 0.0f);
	starting_segment.scale = vec2(map_max_x, map_max_z);
	dynarray_add(map_segments, &starting_segment);

	for(u32 road_index = 0; road_index < map_road_segments->len; road_index++)
	{
		map_road_segment road_seg = *(map_road_segment*) dynarray_get(map_road_segments, road_index);

		// the bounds of the road
		u32 road_min_x = (u32) road_seg.pos.x;
		u32 road_max_x = (u32) road_seg.pos.x + (u32) road_seg.scale.x;
		u32 road_min_z = (u32) road_seg.pos.y;
		u32 road_max_z = (u32) road_seg.pos.y + (u32) road_seg.scale.y;

		// split along the length of the road
		if(road_seg.scale.x == MAP_GEN_ROAD_WIDTH)
		{
			split_segments_on_point(road_min_x, true);
			split_segments_on_point(road_max_x, true);
		}
		else
		{
			split_segments_on_point(road_min_z, false);
			split_segments_on_point(road_max_z, false);
		}
	}

	// remove the segments under the roads
	for(u32 road_index = 0; road_index < map_road_segments->len; road_index++)
	{
		map_road_segment road_seg = *(map_road_segment*) dynarray_get(map_road_segments, road_index);

		// the bounds of the road
		u32 road_min_x = (u32) road_seg.pos.x;
		u32 road_max_x = (u32) road_seg.pos.x + (u32) road_seg.scale.x;
		u32 road_min_z = (u32) road_seg.pos.y;
		u32 road_max_z = (u32) road_seg.pos.y + (u32) road_seg.scale.y;

		for(u32 i = 0; i < map_segments->len; i++)
		{
			map_segment map_seg = *(map_segment*) dynarray_get(map_segments, i);

			// the bounds of the segment
			u32 map_min_x = (u32) map_seg.pos.x;
			u32 map_max_x = (u32) map_seg.pos.x + (u32) map_seg.scale.x;
			u32 map_min_z = (u32) map_seg.pos.y;
			u32 map_max_z = (u32) map_seg.pos.y + (u32) map_seg.scale.y;

			if(map_min_x >= road_min_x && map_max_x <= road_max_x && map_min_z >= road_min_z && map_max_z <= road_max_z)
			{
				dynarray_remove(map_segments, i);
				i--;
			}
		}
	}
}

void map_gen_simplify_segments()
{
	dynarray* fully_simplified_indexes = dynarray_create(map_segments->len, sizeof(s32));
	while(fully_simplified_indexes->len < map_segments->len)
	{
		s32 biggest_segment_index = -1;

		for(u32 i = 0; i < map_segments->len; i++)
		{
			map_segment seg = *(map_segment*) dynarray_get(map_segments, i);

			map_segment biggest_seg;

			if(biggest_segment_index != -1)
			{
				biggest_seg = *(map_segment*) dynarray_get(map_segments, biggest_segment_index);
			}

			if(biggest_segment_index == -1 || seg.scale.x * seg.scale.y > biggest_seg.scale.x * biggest_seg.scale.y)
			{
				// check that this segment hasn't already been fully simplified
				bool already_simplified = false;
				for (u32 j = 0; j < fully_simplified_indexes->len; j++)
				{
					if (*(u32*) dynarray_get(fully_simplified_indexes, j) == i)
					{
						already_simplified = true;
						break;
					}
				}

				if (already_simplified) continue;

				biggest_segment_index = i;
			}
		}

		if(biggest_segment_index < 0) break;

		map_segment biggest_segment = *(map_segment*) dynarray_get(map_segments, biggest_segment_index);

		u32 biggest_min_x = (u32) biggest_segment.pos.x;
		u32 biggest_max_x = (u32) biggest_segment.pos.x + (u32) biggest_segment.scale.x;
		u32 biggest_min_z = (u32) biggest_segment.pos.y;
		u32 biggest_max_z = (u32) biggest_segment.pos.y + (u32) biggest_segment.scale.y;

		for(u32 i = 0; i < map_segments->len; i++)
		{
			if(i + 1 == map_segments->len) dynarray_add(fully_simplified_indexes, &biggest_segment_index);
			if(i == biggest_segment_index) continue;

			map_segment map_seg = *(map_segment*) dynarray_get(map_segments, i);

			u32 map_min_x = (u32) map_seg.pos.x;
			u32 map_max_x = (u32) map_seg.pos.x + (u32) map_seg.scale.x;
			u32 map_min_z = (u32) map_seg.pos.y;
			u32 map_max_z = (u32) map_seg.pos.y + (u32) map_seg.scale.y;

			bool match_along_x = map_seg.scale.y == biggest_segment.scale.y;
			bool match_along_z = map_seg.scale.x == biggest_segment.scale.x;

			if(match_along_z)
			{
				if((biggest_max_z == map_min_z || biggest_min_z == map_max_z) && biggest_min_x == map_min_x && biggest_max_x == map_max_x)
				{
					map_segment merged_segment = {};
					merged_segment.pos = vec2(biggest_segment.pos.x, glm::min(biggest_segment.pos.y, map_seg.pos.y));
					merged_segment.scale = vec2(biggest_segment.scale.x, biggest_segment.scale.y + map_seg.scale.y);

					// erase both the merging segments
					dynarray_remove(map_segments, (u32) biggest_segment_index);
					dynarray_remove(map_segments, ((u32) biggest_segment_index < i ? i - 1 : i)); // index might have changed due to the last erase

					// add the new merged segment
					dynarray_add(map_segments, &merged_segment);
					dynarray_clear(fully_simplified_indexes);
					break;
				}
			}
			else if(match_along_x)
			{
				if((biggest_max_x == map_min_x || biggest_min_x == map_max_x) && biggest_min_z == map_min_z && biggest_max_z == map_max_z)
				{
					map_segment merged_segment = {};
					merged_segment.pos = vec2(glm::min(biggest_segment.pos.x, map_seg.pos.x), biggest_segment.pos.y);
					merged_segment.scale = vec2(biggest_segment.scale.x + map_seg.scale.x, biggest_segment.scale.y);

					// erase both the merging segments
					dynarray_remove(map_segments, (u32) biggest_segment_index);
					dynarray_remove(map_segments, ((u32) biggest_segment_index < i ? i - 1 : i)); // index might have changed due to the last erase

					// add the new merged segment
					dynarray_add(map_segments, &merged_segment);
					dynarray_clear(fully_simplified_indexes);
					break;
				}
			}
		}
	}
}

void add_mini_segments(vec2 pos, vec2 scale)
{
	map_segment seg = {};
	seg.pos = pos;
	seg.scale = scale;
	dynarray_add(map_segments, &seg);
}

void split_segments_on_point(u32 split_point, bool split_on_x)
{
	// @Speed: we could improve performance by making one of these once and just clearing it
	dynarray* to_remove = dynarray_create(map_segments->len, sizeof(u32));

	u32 segments_size = map_segments->len;
	for(u32 map_index = 0; map_index < segments_size; map_index++)
	{
		map_segment map_seg = *(map_segment*) dynarray_get(map_segments, map_index);

		// the bounds of the segment
		u32 map_min_x = (u32) map_seg.pos.x;
		u32 map_max_x = (u32) map_seg.pos.x + (u32) map_seg.scale.x;
		u32 map_min_z = (u32) map_seg.pos.y;
		u32 map_max_z = (u32) map_seg.pos.y + (u32) map_seg.scale.y;

		if(split_on_x)
		{
			if(split_point > map_min_x && split_point < map_max_x)
			{
				add_mini_segments(map_seg.pos, vec2(split_point - map_seg.pos.x, map_seg.scale.y));
				add_mini_segments(vec2(map_seg.pos.x + (split_point - map_seg.pos.x), map_seg.pos.y), vec2(map_seg.scale.x - (split_point - map_seg.pos.x), map_seg.scale.y));

				dynarray_add(to_remove, &map_index);
			}
		}
		else
		{
			if(split_point > map_min_z && split_point < map_max_z)
			{
				map_segment seg1 = {};
				seg1.pos = map_seg.pos;
				seg1.scale = vec2(map_seg.scale.x, split_point - map_seg.pos.y);
				dynarray_add(map_segments, &seg1);

				map_segment seg2 = {};
				seg2.pos = vec2(map_seg.pos.x, seg1.pos.y + seg1.scale.y);
				seg2.scale = vec2(map_seg.scale.x, map_seg.scale.y - seg1.scale.y);
				dynarray_add(map_segments, &seg2);

				dynarray_add(to_remove, &map_index);
			}
		}
	}

	for(u32 i = 0; i < to_remove->len; i++)
	{
		dynarray_remove(map_segments, (*(u32*) dynarray_get(to_remove, i) - i));
	}

	dynarray_destory(to_remove);
}

void map_gen_biomes()
{
	for(u32 i = 0; i < map_segments->len; i++)
	{
		map_segment seg = *(map_segment*) dynarray_get(map_segments, i);

		// must be > 20 on each side and not be more than 2x wide than long and vise versa
		if(seg.scale.x > 20 && seg.scale.y > 20 && max(seg.scale.x, seg.scale.y) / min(seg.scale.x, seg.scale.y) <= 3.0f)
		{
			map_gen_building(seg);
		}
	}
}

void map_gen_building(map_segment seg)
{
	// @Todo: have min and max pad????
	u32 x_pad_1 = max(2, (((double) rand() / RAND_MAX)) * (seg.scale.x / 4.0f));
	u32 x_pad_2 = max(2, (((double)rand() / RAND_MAX)) * (seg.scale.x / 4.0f));
	u32 z_pad_1 = max(2, (((double) rand() / RAND_MAX)) * (seg.scale.y / 4.0f));
	u32 z_pad_2 = max(2, (((double)rand() / RAND_MAX)) * (seg.scale.y / 4.0f));

	building_coverage += (seg.scale.x - x_pad_1 - x_pad_2) * (seg.scale.y - z_pad_1 - z_pad_2);
	building_count++;

	// walls along x
	for (u32 x = seg.pos.x + x_pad_1; x <= seg.pos.x + seg.scale.x - x_pad_2; x++)
	{
		// bottom x wall
		map_add_cover(vec3(x, 0.0f, seg.pos.y + z_pad_1));

		// top x wall
		map_add_cover(vec3(x, 0.0f, seg.pos.y + seg.scale.y - z_pad_2));
	}

	// wall along z
	for (u32 z = seg.pos.y + z_pad_1; z < seg.pos.y + seg.scale.y - z_pad_2; z++)
	{
		// bottom z wall
		map_add_cover(vec3(seg.pos.x + x_pad_1, 0.0f, z));

		// top z wall
		map_add_cover(vec3(seg.pos.x + seg.scale.x - x_pad_2, 0.0f, z));
	}
}