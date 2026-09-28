#pragma once
#include <stdint.h>
typedef enum { INPUT_KEYBOARD, INPUT_MOUSE, INPUT_TOUCHPAD, INPUT_GAMEPAD } input_type_t;
typedef struct { input_type_t type; uint16_t code; int32_t value; uint32_t flags; } input_event_t;
int input_init(void); void input_emit(const input_event_t *event);
