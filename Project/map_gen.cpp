#include "map_gen.h"

#include "map.h"

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
#define MAP_GEN_MIN_ROAD_LENGTH 10

void map_gen_roads();
void map_gen_segments();

void map_gen()
{
	// make sure we have a clean slate
	map_road_segments.clear();
	map_segments.clear();

	// first we build the roads, and then the rest of the generation revolves around this
	map_gen_roads();

	map_gen_segments();
}

// @Todo: pass in map object?
void map_gen_roads()
{
	vec3 start = vec3(0.0f, 0.0f, map_max_z - 1);
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
			bool change_direction = random <= 0.05f;
			if(change_direction || end_of_road)
			{
				road_segment seg = {};
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

				map_road_segments.push_back(seg);

				current_segment_length = 0;
				current_segment_start = progress;

				if(end_of_road) break;
			}
		}
		
		progress += direction;
	}
}

void split_segment_on_point()
{

}

void split_segments_on_point(u32 split_point, bool split_on_x)
{
	std::vector<u32> to_remove;

	u32 segments_size = map_segments.size();
	for(u32 map_index = 0; map_index < segments_size; map_index++)
	{
		map_segment map_seg = map_segments[map_index];

		// the bounds of the segment
		u32 map_min_x = map_seg.pos.x;
		u32 map_max_x = map_seg.pos.x + map_seg.scale.x;
		u32 map_min_z = map_seg.pos.y;
		u32 map_max_z = map_seg.pos.y + map_seg.scale.y;

		if(split_on_x)
		{
			if(split_point > map_min_x && split_point < map_max_x)
			{
				map_segment seg1 = {};
				seg1.pos = map_seg.pos;
				seg1.scale = vec2(split_point - map_seg.pos.x, map_seg.scale.y);
				map_segments.push_back(seg1);

				map_segment seg2 = {};
				seg2.pos = vec2(seg1.pos.x + seg1.scale.x, map_seg.pos.y);
				seg2.scale = vec2(map_seg.scale.x - seg1.scale.x, map_seg.scale.y);
				map_segments.push_back(seg2);

				to_remove.push_back(map_index);
			}
		}
		else
		{
			if(split_point > map_min_z && split_point < map_max_z)
			{
				map_segment seg1 = {};
				seg1.pos = map_seg.pos;
				seg1.scale = vec2(map_seg.scale.x, split_point - map_seg.pos.y);
				map_segments.push_back(seg1);

				map_segment seg2 = {};
				seg2.pos = vec2(map_seg.pos.x, seg1.pos.y + seg1.scale.y);
				seg2.scale = vec2(map_seg.scale.x, map_seg.scale.y - seg1.scale.y);
				map_segments.push_back(seg2);

				to_remove.push_back(map_index);
			}
		}
	}

	for(u32 i = 0; i < to_remove.size(); i++)
	{
		map_segments.erase(map_segments.begin() + (to_remove[i] - i));
	}
}

void map_gen_simplify_segments()
{
	std::vector<u32> fully_simplified_indexes;
	while(fully_simplified_indexes.size() < map_segments.size())
	{
		u32 biggest_segment_index = -1;

		for(u32 i = 0; i < map_segments.size(); i++)
		{
			map_segment seg = map_segments[i];

			if(biggest_segment_index == -1 || seg.scale.x * seg.scale.y > map_segments[biggest_segment_index].scale.x * map_segments[biggest_segment_index].scale.y)
			{
				// check that this segment hasn't already been fully simplified
				bool already_simplified = false;
				for(u32 j = 0; j < fully_simplified_indexes.size(); j++)
				{
					if(fully_simplified_indexes[j] == i)
					{
						already_simplified = true;
						break;
					}
				}

				if(already_simplified) continue;

				biggest_segment_index = i;
			}
		}

		if(biggest_segment_index < 0) break;

		map_segment biggest_segment = map_segments[biggest_segment_index];

		u32 biggest_min_x = biggest_segment.pos.x;
		u32 biggest_max_x = biggest_segment.pos.x + biggest_segment.scale.x;
		u32 biggest_min_z = biggest_segment.pos.y;
		u32 biggest_max_z = biggest_segment.pos.y + biggest_segment.scale.y;

		for(u32 i = 0; i < map_segments.size(); i++)
		{
			if(i + 1 == map_segments.size()) fully_simplified_indexes.push_back(biggest_segment_index);
			if(i == biggest_segment_index) continue;

			map_segment map_seg = map_segments[i];

			u32 map_min_x = map_seg.pos.x;
			u32 map_max_x = map_seg.pos.x + map_seg.scale.x;
			u32 map_min_z = map_seg.pos.y;
			u32 map_max_z = map_seg.pos.y + map_seg.scale.y;

			bool match_along_x = map_seg.scale.y == biggest_segment.scale.y;
			bool match_along_z = map_seg.scale.x == biggest_segment.scale.x;

			if(match_along_x)
			{
				if((biggest_max_x == map_min_x || biggest_min_x == map_max_x) && biggest_min_z == map_min_z && biggest_max_z == map_max_z)
				{
					map_segment merged_segment = {};
					merged_segment.pos = vec2(glm::min(biggest_segment.pos.x, map_seg.pos.x), biggest_segment.pos.y);
					merged_segment.scale = vec2(biggest_segment.scale.x + map_seg.scale.x, biggest_segment.scale.y);

					// erase both the merging segments
					map_segments.erase(map_segments.begin() + biggest_segment_index);
					map_segments.erase(map_segments.begin() + (biggest_segment_index < i ? i - 1 : i)); // index might have changed due to the last erase

					// add the new merged segment
					map_segments.push_back(merged_segment);
					fully_simplified_indexes.clear();

					printf("merged\n");
					break;
				}
			}
			else if(match_along_z)
			{
				if((biggest_max_z == map_min_z || biggest_min_z == map_max_z) && biggest_min_x == map_min_x && biggest_max_x == map_max_x)
				{
					map_segment merged_segment = {};
					merged_segment.pos = vec2(biggest_segment.pos.x, glm::min(biggest_segment.pos.y, map_seg.pos.y));
					merged_segment.scale = vec2(biggest_segment.scale.x, biggest_segment.scale.y + map_seg.scale.y);

					// erase both the merging segments
					map_segments.erase(map_segments.begin() + biggest_segment_index);
					map_segments.erase(map_segments.begin() + (biggest_segment_index < i ? i - 1 : i)); // index might have changed due to the last erase

					// add the new merged segment
					map_segments.push_back(merged_segment);
					fully_simplified_indexes.clear();

					printf("merged\n");
					break;
				}
			}
		}
	}
}

void map_gen_segments()
{
	map_segment starting_segment = {};
	starting_segment.pos = vec2(0.0f, 0.0f);
	starting_segment.scale = vec2(map_max_x, map_max_z);
	map_segments.push_back(starting_segment);

	for(u32 road_index = 0; road_index < map_road_segments.size(); road_index++)
	{
		road_segment road_seg = map_road_segments[road_index];

		// the bounds of the road
		u32 road_min_x = road_seg.pos.x;
		u32 road_max_x = road_seg.pos.x + road_seg.scale.x;
		u32 road_min_z = road_seg.pos.y;
		u32 road_max_z = road_seg.pos.y + road_seg.scale.y;

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
	for(u32 road_index = 0; road_index < map_road_segments.size(); road_index++)
	{
		road_segment road_seg = map_road_segments[road_index];

		// the bounds of the road
		u32 road_min_x = road_seg.pos.x;
		u32 road_max_x = road_seg.pos.x + road_seg.scale.x;
		u32 road_min_z = road_seg.pos.y;
		u32 road_max_z = road_seg.pos.y + road_seg.scale.y;

		for(u32 i = 0; i < map_segments.size(); i++)
		{
			map_segment map_seg = map_segments[i];

			// the bounds of the segment
			u32 map_min_x = map_seg.pos.x;
			u32 map_max_x = map_seg.pos.x + map_seg.scale.x;
			u32 map_min_z = map_seg.pos.y;
			u32 map_max_z = map_seg.pos.y + map_seg.scale.y;

			if(map_min_x >= road_min_x && map_max_x <= road_max_x && map_min_z >= road_min_z && map_max_z <= road_max_z)
			{
				map_segments.erase(map_segments.begin() + i);
				i--;
			}
		}
	}

	map_gen_simplify_segments();
}