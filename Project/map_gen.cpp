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

#define ROAD_WIDTH 5
#define MIN_ROAD_LENGTH 14

#define MIN_BUILDING_SIZE 20

#define BUILDING_MAX_WIDTH_LENGTH_RATIO 3.0f
#define BUILDING_MAX_PAD_RATIO 5.0f

#define MIN_MAX_ROOM_LEN 6

void map_gen_roads();
void map_gen_cars();

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
	if(regen_tries > 0) map_clear_blocks();

	// make sure we have a clean slate
	building_coverage = 0;
	building_count = 0;

	dynarray_clear(map_road_segments);
	dynarray_clear(map_segments);

	// first we build the roads, and then the rest of the generation revolves around this
	map_gen_roads();

	map_gen_segments();
	map_gen_simplify_segments();

	map_gen_biomes();

	map_gen_cars();

	// regen if we have a bad building coverage
	if ((building_coverage / (double) (map_max_x * map_max_z) < 0.1f || building_count < 2) && regen_tries <= 50)
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

		if(current_segment_length++ > MIN_ROAD_LENGTH || end_of_road)
		{
			double random = ((double) rand() / (double) RAND_MAX);

			// randomly change direction, but only if you're not too close to the edge of the map
			bool change_direction = random <= 0.075f;

			// check if we're close to the end (cast to s32 just incase MAP_GEN_MIN_ROAD_LENGTH - MAP_GEN_ROAD_WIDTH - 1 > map_max_x (and then we would wrap back to 2.4bil)
			bool close_to_end = progress.x >= (s32) map_max_x - MIN_ROAD_LENGTH - ROAD_WIDTH - 1 || progress.z <= MIN_ROAD_LENGTH + ROAD_WIDTH + 1;

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
					
					seg.scale = vec2(ROAD_WIDTH, current_segment_length);
					seg.pos.y -= current_segment_length - 1;
				}
				else
				{
					direction.x = 0.0f;
					direction.z = -1.0f;

					seg.scale = vec2(current_segment_length, ROAD_WIDTH);
					seg.pos.y -= ROAD_WIDTH - 1;
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

void map_gen_cars()
{
	s32 car_width = 3;
	s32 car_length = 5;
	s32 min_car_padding = 1;

	for (u32 road_index = 0; road_index < map_road_segments->len; road_index++)
	{
		map_road_segment road_seg = *(map_road_segment*) dynarray_get(map_road_segments, road_index);

		// the bounds of the road
		u32 road_min_x = (u32) road_seg.pos.x;
		u32 road_max_x = (u32) road_seg.pos.x + (u32) road_seg.scale.x;
		u32 road_min_z = (u32) road_seg.pos.y;
		u32 road_max_z = (u32) road_seg.pos.y + (u32) road_seg.scale.y;

		if(road_seg.scale.x > road_seg.scale.y)
		{
			s32 last_car = road_seg.pos.x;

			if(road_index > 0) last_car = -car_length - 1;

			u32 current_pos = 0;

			while (true)
			{
				if (current_pos++ < last_car + car_length + min_car_padding)
				{
					current_pos = last_car + car_length + min_car_padding;
				}

				if (current_pos + car_length >= road_max_x - road_min_x) break;

				double random = (double) rand() / RAND_MAX;

				if (random < 0.2)
				{
					last_car = current_pos;

					for (u32 x = 0; x < car_length; x++)
					{
						for (u32 z = 0; z < car_width; z++)
						{
							map_set_cover(vec3(road_min_x + current_pos + x + 1, 0.0f, road_min_z + z + 1), x == car_length - 1 ? 1 : 2);
						}
					}
				}
			}
		}
		else
		{
			s32 last_car = road_max_z;

			if (road_index > 0) last_car = road_max_z;

			s32 current_pos = road_max_z;

			while (true)
			{
				if (current_pos-- > last_car - car_length - min_car_padding)
				{
					current_pos = last_car - car_length - min_car_padding;
				}

				if (current_pos < (s32) road_min_z) break;

				double random = (double) rand() / RAND_MAX;

				if (random < 0.2)
				{
					last_car = current_pos;

					for (u32 x = 0; x < car_width; x++)
					{
						for (u32 z = 0; z < car_length; z++)
						{
							map_set_cover(vec3(road_min_x + x + 1, 0.0f, current_pos + z), z == 0 ? 1 : 2);
						}
					}
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
		if(road_seg.scale.x == ROAD_WIDTH)
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
					merged_segment.pos = vec2(biggest_segment.pos.x, min(biggest_segment.pos.y, map_seg.pos.y));
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
					merged_segment.pos = vec2(min(biggest_segment.pos.x, map_seg.pos.x), biggest_segment.pos.y);
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

		// must be > 20 on each side and not be more than 3x wide than long and vise versa
		if(seg.scale.x > MIN_BUILDING_SIZE && seg.scale.y > MIN_BUILDING_SIZE
			&& max(seg.scale.x, seg.scale.y) / min(seg.scale.x, seg.scale.y) <= BUILDING_MAX_WIDTH_LENGTH_RATIO)
		{
			map_gen_building(seg);
		}
	}
}

void map_gen_building(map_segment seg)
{
	u32 x_pad_1 = max((u32) 2, (u32) ((((double) rand() / RAND_MAX)) * (seg.scale.x / BUILDING_MAX_PAD_RATIO)));
	u32 x_pad_2 = max((u32) 2, (u32) ((((double) rand() / RAND_MAX)) * (seg.scale.x / BUILDING_MAX_PAD_RATIO)));
	u32 z_pad_1 = max((u32) 2, (u32) ((((double) rand() / RAND_MAX)) * (seg.scale.y / BUILDING_MAX_PAD_RATIO)));
	u32 z_pad_2 = max((u32) 2, (u32) ((((double) rand() / RAND_MAX)) * (seg.scale.y / BUILDING_MAX_PAD_RATIO)));

	u32 building_width = (seg.scale.x - x_pad_1 - x_pad_2);
	u32 building_length = (seg.scale.y - z_pad_1 - z_pad_2);

	building_coverage += building_width * building_length;
	building_count++;

	// walls along x
	for (u32 x = seg.pos.x + x_pad_1; x <= seg.pos.x + seg.scale.x - x_pad_2; x++)
	{
		// bottom x wall
		map_set_cover(vec3(x, 0.0f, seg.pos.y + z_pad_1), 2);

		// top x wall
		map_set_cover(vec3(x, 0.0f, seg.pos.y + seg.scale.y - z_pad_2), 2);
	}

	// wall along z
	for (u32 z = seg.pos.y + z_pad_1; z < seg.pos.y + seg.scale.y - z_pad_2; z++)
	{
		// bottom z wall
		map_set_cover(vec3(seg.pos.x + x_pad_1, 0.0f, z), 2);

		// top z wall
		map_set_cover(vec3(seg.pos.x + seg.scale.x - x_pad_2, 0.0f, z), 2);
	}

	// generate internal walls
	u32 x_len = MIN_MAX_ROOM_LEN + ((double) rand() / RAND_MAX) * (building_width - MIN_MAX_ROOM_LEN * 2);
	u32 z_pos = MIN_MAX_ROOM_LEN + ((double) rand() / RAND_MAX) * (building_length - MIN_MAX_ROOM_LEN * 2);

	// gen first wall
	for (u32 x = seg.pos.x + x_pad_1; x < seg.pos.x + x_pad_1 + x_len; x++)
	{
		map_set_cover(vec3(x, 0.0f, seg.pos.y + z_pad_1 + z_pos), 2);
	}

	// gen second wall along z in a random direction to make a room
	bool left_side_wall = ((double) rand() / RAND_MAX) > 0.5f;
	bool right_side_wall = (((double) rand() / RAND_MAX) > 0.5f) || !left_side_wall;

	if (left_side_wall)
	{
		// gen along left side
		for (u32 z = seg.pos.y + z_pad_1; z <= seg.pos.y + z_pad_1 + z_pos; z++)
		{
			map_set_cover(vec3(seg.pos.x + x_pad_1 + x_len, 0.0f, z), 2);
		}
	}

	if(right_side_wall)
	{
		// gen along right side
		for (u32 z = seg.pos.y + z_pad_1 + z_pos; z <= seg.pos.y + z_pad_1 + building_length; z++)
		{
			map_set_cover(vec3(seg.pos.x + x_pad_1 + x_len, 0.0f, z), 2);
		}
	}

	// gen external doors
	u32 door_x_left;

	while ((door_x_left = 2 + (((double) rand() / RAND_MAX)) * (building_width - 4)) < x_len + 2 && door_x_left > x_len - 2 && left_side_wall);
	map_set_cover(vec3(seg.pos.x + x_pad_1 + door_x_left, 0.0f, seg.pos.y + z_pad_1), 0);

	u32 door_x_right;

	while ((door_x_right = 2 + (((double) rand() / RAND_MAX)) * (building_width - 4)) < x_len + 2 && door_x_right > x_len - 2 && right_side_wall);
	map_set_cover(vec3(seg.pos.x + x_pad_1 + door_x_right, 0.0f, seg.pos.y + z_pad_1 + building_length), 0);

	// gen internal doors
	bool door_on_first_internal = ((double) rand() / RAND_MAX) > 0.5f;

	if(door_on_first_internal)
	{
		map_set_cover(vec3(seg.pos.x + x_pad_1 + 2, 0.0f, seg.pos.y + z_pad_1 + z_pos), 0);
	}

	if(!door_on_first_internal || (left_side_wall && right_side_wall))
	{
		if(left_side_wall)
		{
			u32 wall = 2 + (((double) rand() / RAND_MAX)) * (z_pos - 4);
			map_set_cover(vec3(seg.pos.x + x_pad_1 + x_len, 0.0f, seg.pos.y + z_pad_1 + wall), 0);
		}

		if (right_side_wall)
		{
			u32 wall = 2 + (((double) rand() / RAND_MAX)) * (building_length - z_pos - 4);
			map_set_cover(vec3(seg.pos.x + x_pad_1 + x_len, 0.0f, seg.pos.y + z_pad_1 + building_length - wall), 0);
		}
	}
}