#pragma once

#include "dynstr.h"
#include "gui.h"
#include "action.h"

#include <vector>

void actionbar_init();
void actionbar_update(float dt);
void actionbar_draw();

void actionbar_switch_off_mode(action_mode old_mode);

void actionbar_set_msg(char* msg, float show_seconds);
void actionbar_combatlog_add(char* format, ...);

void action_move_mode(button* this_button);
void action_shoot_mode(button* this_button);
void action_do_nothing(button* this_button);