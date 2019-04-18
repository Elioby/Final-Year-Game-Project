$input v_texcoord0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.sh"

SAMPLER2D(textureSampler, 0);

uniform vec4 tint_color;

void main()
{
	gl_FragColor = vec4(tint_color.x, tint_color.y, tint_color.z, texture2D(textureSampler, v_texcoord0).w * tint_color.w);
}
