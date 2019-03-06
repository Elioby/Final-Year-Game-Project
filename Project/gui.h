#pragma once

#include "general.h"
#include "image.h"
#include "font.h"
#include "dynstr.h"

struct button
{
	u32 x;
	u32 y;
	u32 width;
	u32 height;

	image* icon_img;

	image* bg_img;
	image* hover_bg_img;

	void (*click_callback)();
};


void gui_init();
void gui_update();
void gui_end_frame();

// Checks if a click was already handled by the gui (must be called after gui_update)
bool gui_handled_click();

void gui_draw_image(image* image, u32 x, u32 y, u32 width, u32 height);
void gui_draw_image(image* image, mat4 transform_matrix);
void gui_draw_button(button button);

void gui_draw_text(font* font, char* text, u16 text_len, u32 x, u32 y, float scale);
void gui_draw_text(font* font, char* text, u32 x, u32 y, float scale);
void gui_draw_text(font* font, dynstr* text, u32 x, u32 y, float scale);

button* gui_create_button();
