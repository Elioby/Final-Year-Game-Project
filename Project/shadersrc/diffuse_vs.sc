$input a_position, a_normal
$output v_fragpos, v_normal

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_fragpos = vec3(u_model[0] * vec4(a_position, 1.0f));
	v_normal = a_normal;
}
