#pragma once
#include <stdint.h>
typedef enum { DRIVER_UNKNOWN, DRIVER_STORAGE, DRIVER_NETWORK, DRIVER_USB, DRIVER_AUDIO, DRIVER_INPUT, DRIVER_BLUETOOTH, DRIVER_GPU } driver_class_t;
typedef struct driver { const char *name; driver_class_t class; int (*init)(void); int (*probe)(void *device); } driver_t;
