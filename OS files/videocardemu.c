/*# Copyright 2025 SWAGNER
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.*/
#include "videocardemu.h"
#include <stdio.h>

static uint8_t screen[HEIGHT][WIDTH];

// Символы для псевдографики 
static const char* grayscale = " $.:-=+*#%@<>?abcdefghijklmnopqrstuvwxyz";

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
            int color_idx = screen[y][x] * 39 / 255;
            printf("%c", grayscale[color_idx]);
        }
        printf("\n");
    }
    printf("VGA\n");
}