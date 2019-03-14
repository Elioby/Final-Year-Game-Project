#pragma once

#include "dynstr.h"
#include "gui.h"

#include <vector>

void actionbar_init();
void actionbar_update(float dt);
void actionbar_draw();
void actionbar_set_msg(char* msg, float show_seconds);
void actionbar_combatlog_add(char* format, ...);