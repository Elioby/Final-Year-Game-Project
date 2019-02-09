#pragma once

#include "general.h"
#include "image.h"

struct button
{
	u32 x;
	u32 y;
	u32 width;
	u32 height;

	image icon_img;

	image bg_img;
	image hover_bg_img;

	void (*click_callback)();
};

void gui_init();
bool gui_update();

void gui_draw_image(image image, u32 x, u32 y, u32 width, u32 height);
void gui_draw_image(image image, mat4 transform_matrix);
void gui_draw_button(button button);

button* gui_create_button();
