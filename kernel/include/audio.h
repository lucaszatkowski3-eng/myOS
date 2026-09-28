#pragma once
#include <stdint.h>
int audio_init(void); int audio_play_pcm(const void *data,uint32_t bytes,uint32_t rate,uint8_t channels);
