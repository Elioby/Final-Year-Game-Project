#include "map.h"

#include <stdlib.h>
#include <emmintrin.h>
#include <libmorton/morton.h>
#include <time.h>

#include "mesh.h"
#include "entity.h"
#include "asset_manager.h"
#include "debug.h"
#include "map_gen.h"

u32 map_max_x = 96;
u32 map_max_z = 128;

// @Todo: change "cover_at_block" to an array of tiles, each referencing either nothing, an entity, or cover 
//         (this means you get O(n) lookup of whats in a block based on it's position)
// @Todo: make a map object that holds tile array

bool* cover_at_block;

std::vector<map_segment> map_segments;
std::vector<road_segment> map_road_segments;

void map_init()
{
	// @Todo: move this to some random util file???
	srand(time(0));

	map_gen();

	cover_at_block = (bool*) debug_calloc(pow(max(map_max_x, map_max_z), 2), sizeof(bool));

	u32 friendly_count = 4;
	for(u32 i = 0; i < friendly_count; i++) 
	{
		entity_add(vec3(i * 2 + 1 + map_max_x / 2 - friendly_count, 0, 2), TEAM_FRIENDLY);
	}

	u32 enemy_count = 4;
	for(u32 i = 0; i < enemy_count; i++) 
	{
		entity_add(vec3(i * 2 + 1 + map_max_x / 2 - enemy_count, 0, map_max_z - 2), TEAM_ENEMY);
	}
}

void map_draw()
{
	// draw cover
	for(u32 x = 0; x < map_max_x; x++)
	{
		for(u32 z = 0; z < map_max_z; z++)
		{
			bool cover = map_is_cover_at_block(vec3(x, 0.0f, z));
			if(cover)
			{
				graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(vec3(x, 0.0f, z), 0.0f, vec3(1.0f), vec3(1.0f, 2.0f, 1.0f)));
			}
		}
	}

	// draw road
	for(u32 i = 0; i < map_road_segments.size(); i++)
	{
		road_segment seg = map_road_segments[i];
		graphics_draw_mesh(asset_manager_get_mesh("cube"),
			graphics_create_model_matrix(vec3(seg.pos.x, 0.0f, seg.pos.y), 0.0f, vec3(1.0f), vec3(seg.scale.x, 0.1f, seg.scale.y)), vec4(0.3f, 0.3f, 0.3f, 1.0f));
	}

	u32 color_progress = 0;

	// temp draw segments
	for(u32 i = 0; i < map_segments.size(); i++)
	{
		map_segment seg = map_segments[i];

		u32 current_color = color_progress += (255 * 255 * 255) / (map_segments.size() / 0.8f);
		u8 r = *((u8*) &current_color);
		u8 g = *((u8*) &current_color + 1);
		u8 b = *((u8*) &current_color + 2);

		vec4 seg_color = vec4((float) r / 255.0f, (float) g / 255.0f, (float) b / 255.0f, 1.0f);

		graphics_draw_mesh(asset_manager_get_mesh("cube"), 
			graphics_create_model_matrix(vec3(seg.pos.x, 0.0f, seg.pos.y), 0.0f, vec3(1.0f), vec3(seg.scale.x, 0.2f, seg.scale.y)), seg_color);
	}

	// draw entites and healthbars
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* ent = entities[i];

		if(!ent->dead)
		{
			vec3 healthbox_aspect = vec3(1.0f, 1.0f / 3.0f, 1.0f);

			if(ent->health > 0)
			{
				image* img;
				if(ent->team == TEAM_ENEMY) img = asset_manager_get_image("enemy_healthbar");
				else img = asset_manager_get_image("friendly_healthbar");

				graphics_draw_image(img, graphics_create_model_matrix(vec3(ent->pos.x + 0.033333f, ent->pos.y + 2.038f, ent->pos.z + 0.5f), 0.0f, vec3(1.0f),
					vec3((0.5f - 0.1f / 3.0f) * (ent->health / (float)ent->max_health), 0.1285f, 1.0f)));
			}

			graphics_draw_image(asset_manager_get_image("healthbox"), graphics_create_model_matrix(vec3(ent->pos.x, ent->pos.y + 2.0f, ent->pos.z + 0.5f), 0.0f, vec3(1.0f), healthbox_aspect * 0.5f));

			graphics_draw_mesh(ent->mesh, graphics_create_model_matrix(ent->pos, 0.0f, vec3(1.0f), vec3(1.0f)));
		}
	}

	// draw terrain
	graphics_draw_mesh(asset_manager_get_mesh("terrain"), graphics_create_model_matrix(vec3(0.0f), 0.0f, vec3(1.0f), vec3(1.0f)));
}

void map_add_cover(vec3 block_pos)
{
	cover_at_block[libmorton::morton2D_32_encode((u32) block_pos.x, (u32) block_pos.z)] = true;
}

bool map_is_cover_at_block(u32 x, u32 z)
{
	uint_fast32_t i = libmorton::morton2D_32_encode(x, z);
	return cover_at_block[i];
}

bool map_is_cover_at_block(vec3 block_pos)
{
	if (block_pos.x < 0 || block_pos.z < 0  || block_pos.x >= map_max_x || block_pos.z >= map_max_z) return false;
	return map_is_cover_at_block((u32) block_pos.x, (u32) block_pos.z);
}

vec3 map_get_adjacent_cover(vec3 start, vec3 closest_to)
{
	float smallest_distance = +FLT_MAX;
	vec3 closest_cover = vec3(-1.0f);

	for(i8 x = -1; x <= 1; x++)
	{
		for(i8 z = -1; z <= 1; z++)
		{
			vec3 block_pos = start;
			block_pos.x += x;
			block_pos.z += z;

			float x_delta = abs(block_pos.x - closest_to.x);
			float z_delta = abs(block_pos.z - closest_to.z);

			float distance = (x_delta * x_delta) + (z_delta * z_delta);
			if(smallest_distance > distance && map_is_cover_at_block(block_pos))
			{
				closest_cover = block_pos;
				smallest_distance = distance;
			}
		}
	}

	return closest_cover;
}

bool map_has_los_internal(float start_x, float start_z, float end_x, float end_z)
{
	vec2 direction = glm::normalize(vec2(end_x - start_x, end_z - start_z));

	start_x += 0.5f;
	start_z += 0.5f;

	// v * inversesqrt(dot(v, v));

	float step_x = direction.x * MAP_RAYTRACE_ACCURACY;
	float step_z = direction.y * MAP_RAYTRACE_ACCURACY;

	long last_block_x = -1;
	long last_block_z = -1;
	long next_block_x;
	long next_block_z;

	while(true)
	{
		start_x += step_x;
		start_z += step_z;

		next_block_x = (long) start_x;
		next_block_z = (long) start_z;

		// only eval if this is a new block than last
		if(!(next_block_x == last_block_x && next_block_z == last_block_z))
		{
			// if we reached the target, we have full los
			if(next_block_x == end_x && next_block_z == end_z) return true;

			// if this block is off the map, they have no los
			if(next_block_x < 0 || next_block_z < 0) return false;

			// if this block is cover, they have no los
			int_fast16_t index = libmorton::morton2D_32_encode(next_block_x, next_block_z);
			if(cover_at_block[index]) return false;

			last_block_x = next_block_x;
			last_block_z = next_block_z;
		}
	}

	return false;
}

bool map_has_los(entity* ent1, entity* ent2)
{
	// how leinient is the los algorithm, in blocks
	float lean_distance = 0.5f;

	// @Todo: this is a bit much, maybe we only need to check along the opposite axis of the direction
	return map_has_los_internal(ent1->pos.x, ent1->pos.z, ent2->pos.x, ent2->pos.z) ||
		map_has_los_internal(ent1->pos.x + lean_distance, ent1->pos.z, ent2->pos.x, ent2->pos.z) ||
		map_has_los_internal(ent1->pos.x - lean_distance, ent1->pos.z, ent2->pos.x, ent2->pos.z) ||
		map_has_los_internal(ent1->pos.x, ent1->pos.z + lean_distance, ent2->pos.x, ent2->pos.z) ||
		map_has_los_internal(ent1->pos.x, ent1->pos.z - lean_distance, ent2->pos.x, ent2->pos.z) ||
		map_has_los_internal(ent2->pos.x + lean_distance, ent2->pos.z, ent1->pos.x, ent1->pos.z) ||
		map_has_los_internal(ent2->pos.x - lean_distance, ent2->pos.z, ent1->pos.x, ent1->pos.z) ||
		map_has_los_internal(ent2->pos.x, ent2->pos.z + lean_distance, ent1->pos.x, ent1->pos.z) ||
		map_has_los_internal(ent2->pos.x, ent2->pos.z - lean_distance, ent1->pos.x, ent1->pos.z);
}

float map_get_los_angle(entity* inflict_ent, entity* target_ent)
{
	float los_amount = 1.0f;

	// @Todo: this is kinda bad but im tired (The way we use -1.0f as "no adjacent cover")
	vec3 closest_cover = map_get_adjacent_cover(target_ent->pos, inflict_ent->pos);

	if(!map_pos_equal(closest_cover, vec3(-1.0f)))
	{
		vec3 cover_to_covered_vector = glm::normalize(closest_cover - target_ent->pos);
		vec3 cover_to_shooter = glm::normalize(inflict_ent->pos - closest_cover);

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