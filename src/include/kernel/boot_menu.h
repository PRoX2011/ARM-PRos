#ifndef KERNEL_BOOT_MENU_H
#define KERNEL_BOOT_MENU_H

typedef struct {
	unsigned width;
	unsigned height;
	unsigned font_h;
	int use_spi;
} boot_menu_result_t;

void boot_menu_run(boot_menu_result_t *result);

#endif
