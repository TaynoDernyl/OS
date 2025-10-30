#include "videocardemu.h"
#include <stdio.h>

static uint8_t screen[HEIGHT][WIDTH];

// Символы для псевдографики (от тёмного к светлому)
static const char* grayscale = " $.:-=+*#%@";

void vga_init() {
    for(int y=0; y<HEIGHT; y++)
        for(int x=0; x<WIDTH; x++)
            screen[y][x] = 0; // чёрный
}

void vga_clear() {
    printf("\033[2J\033[H"); // очистка консоли и курсор в начало
}

void vga_set_pixel(int x, int y, uint8_t color) {
    if(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
        screen[y][x] = color;
}

void vga_render() {
    vga_clear(); // очищаем консоль перед выводом
    
    for(int y=0; y<HEIGHT; y++) {
        for(int x=0; x<WIDTH; x++) {
            // Преобразуем цвет (0-255) в символ (0-10)
            int color_idx = screen[y][x] * 10 / 255;
            printf("%c", grayscale[color_idx]);
        }
        printf("\n");
    }
    printf("VGA\n");
}