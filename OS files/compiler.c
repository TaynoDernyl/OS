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

#define version "0.0.7"

#include <stdio.h> //стандартная библиотека
#include <stdarg.h> //для работы с аргументами в функциях
#include <string.h> //для работы со строками
#include <unistd.h> //для union
#include <stdint.h> //для регистров (uint8_t и т.д.)
#include <stdbool.h> //для операций над false & true

bool trace = false; //трассировка
const char* code = NULL; //определение места под нашу программу
bool save = false; //***-если не сохранено, ===-если сохранено

void parser_stroke(char *stroke){
    for(int i = 0; i <= 30; ++i){
        int symb = getchar(); 
        bool exit = false;
        switch(symb){
            case '\n':
                exit = true;
                break;
            case 24:
                exit = true;
                strcpy(stroke, "");
                if(trace)printf("[TRACE] Exit & clear buffer\n");
                break;
        }
        if (exit == false){
            stroke[i] = symb;
            if (trace){printf("[TRACE] %c, number = %u\n", stroke[i], i);}
        } else{
            break;
        }

    }
    return;
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trace") == 0) trace = 1; //включена ли трассировка
        else code = argv[i]; //есть ли файл при запуске
    }
    if (trace){printf("Hi, this is LETOS compiler, version %s\n", version);}
    if (code == NULL){
        if(save == true){
            printf("===:"); 
        }
        else if(save == false){
            printf("***:");
        }
        while(true){
            char command[30] = {0};
            parser_stroke(command);
            printf("%s", command);
            break;
            //parser

        }
    }
}

