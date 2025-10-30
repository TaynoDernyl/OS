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
#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define WIDTH 80    // Уменьшаем для консоли
#define HEIGHT 12   // Уменьшаем для консоли

void vga_init();
void vga_set_pixel(int x, int y, uint8_t color);
void vga_render(); // вывод в консоль как ASCII-графика
void vga_clear();  // очистка экрана

#endif