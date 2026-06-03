#ifndef DRIVERS_SPI_H
#define DRIVERS_SPI_H

#include <stdint.h>
#include <stddef.h>

void spi0_init(uint32_t clock_divider);
void spi0_write(const uint8_t *data, size_t len);

#endif
