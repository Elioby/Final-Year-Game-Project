#pragma once

typedef enum action_mode {
	ACTION_MODE_SELECT_UNITS,
	ACTION_MODE_MOVE,
	ACTION_MODE_SHOOT,
	ACTION_MODE_THROW
} action_mode_t;

extern action_mode current_action_mode;