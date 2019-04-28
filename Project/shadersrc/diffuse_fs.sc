$input v_fragpos, v_normal

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.sh"

uniform vec4 tint_color;

void main()
{
	vec3 light_color = vec3(0.8f, 0.8f, 0.8f);
	vec3 light_pos = vec3(128.0f, 50.0f, 128.0f);

	/* Ambient lighting */
	float ambient_strength = 0.3f;
	vec3 ambient = ambient_strength * light_color;

	/* Diffuse lighting */
	vec3 normal_dir = normalize(v_normal);
	vec3 light_dir = vec3(0.0f, 1.0f, 0.0f);
	float dot = max(dot(normal_dir, light_dir), 0.0);
	vec3 diffuse = dot * light_color;

	/* Combination */
	vec4 result = vec4(ambient + diffuse, 1.0f) * tint_color;
	gl_FragColor = result;
}
