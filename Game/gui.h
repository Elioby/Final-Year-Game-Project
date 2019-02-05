#pragma once

#include "general.h"
#include "image.h"

struct gui_button
{
	u32 x;
	u32 y;
	u32 width;
	u32 height;

	image img;

	bool has_hover;
	image hover_img;

	void (*click_callback)();
};

void gui_init();

void gui_draw_image(image image, u32 x, u32 y, u32 width, u32 height);
void gui_draw_image(image image, mat4 transform_matrix);
void gui_draw_button(gui_button button);
