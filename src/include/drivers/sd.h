#ifndef _SD_H
#define _SD_H

#include <stdint.h>

int sd_init(void);
int sd_read_sector(uint32_t lba, uint8_t *buffer);
int sd_write_sector(uint32_t lba, uint8_t *buffer);

#endif
