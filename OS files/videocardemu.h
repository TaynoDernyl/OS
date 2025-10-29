#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define WIDTH 80    // Уменьшаем для консоли
#define HEIGHT 40   // Уменьшаем для консоли

void vga_init();
void vga_set_pixel(int x, int y, uint8_t color);
void vga_render(); // вывод в консоль как ASCII-графика
void vga_clear();  // очистка экрана

#endif