#pragma once
#include <stdint.h>
void ata_pio_init(void);
int ata_pio_available(void);
int ata_pio_read(uint32_t lba, void *buffer, uint32_t sectors);
int ata_pio_write(uint32_t lba, const void *buffer, uint32_t sectors);
