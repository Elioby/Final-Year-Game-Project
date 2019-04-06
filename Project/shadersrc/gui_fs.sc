$input v_texcoord0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.sh"

SAMPLER2D(textureSampler, 0);

void main()
{
	gl_FragColor = texture2D(textureSampler, v_texcoord0);
}
