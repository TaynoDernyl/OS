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
#define MAX_LINES 200

#include <stdio.h> //стандартная библиотека
#include <stdarg.h> //для работы с аргументами в функциях
#include <string.h> //для работы со строками
#include <unistd.h> //для union
#include <stdint.h> //для регистров (uint8_t и т.д.)
#include <stdbool.h> //для операций над false & true
#include <stdlib.h> //для системных функций
#include <conio.h> //для консоли

typedef struct{

    int c_x; //координата начала строки по оси x
    int c_y; //координата строки по оси y
    int y_line; //какая строка
    int symbs; //количество символов
    char* command; //строка через | вместо пробелов

} Line; //структура строк

Line lines[MAX_LINES]; //максимальное количество строк

typedef struct{

    char* name;
    uint8_t opcode;
    bool flag;
    

} Register; //структура регистров

Register registers[] = {
    // 8-битные регистры (flag = 0)
    {"al", 0x00, 0},
    {"ah", 0x01, 0},
    {"bl", 0x02, 0},
    {"bh", 0x03, 0},
    {"px", 0x09, 1},
    {"py", 0x0A, 1},
    // 16-битные регистры (flag = 1)
    {"ax", 0x04, 1},
    {"bx", 0x05, 1},
    {"ds", 0x06, 1},
    {"cs", 0x07, 1},
    {"cx", 0x08, 1},
    {"sp", 0x0B, 1},
    {"rt", 0x0C, 1},
    {"pc", 0xFF, 1}
};

typedef struct {

    char* name;
    uint8_t opcode;
    bool if_flag;
    bool if_IR;
    uint8_t opperands; //=======================================================ДОДЕЛАТЬ!!!!!!!

} Command;


bool trace = false; //трассировка
const char* code = NULL; //определение места под нашу программу
bool save = false; //***-если не сохранено, ===-если сохранено

static void load_code_from_input_file(char *name){
    FILE *file = fopen(name, "r");
    if (!file) {perror("fopen"); exit(1);}

}

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void parser_stroke(char *stroke, int *x, int *y){ //разбираем строку на слова с разделителем '|'
    for(int i = 0; i <= 30; ++i){
        int symb = getchar(); 
        *x += 1;
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
            if(symb == ' '){stroke[i] = '|';}
            else{
                stroke[i] = symb;
                }
            if (trace){printf("[TRACE] %c, number = %u\n", stroke[i], i);}
        } else{
            break;
        }

    }
    
    return;
}

bool parser_words(char *words){
    
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trace") == 0) trace = 1; //включена ли трассировка
        else code = argv[i]; //есть ли файл при запуске
    }
    if (trace){printf("Hi, this is LETOS compiler, version %s\n", version);}
    if (true){
        int x = 0; //координата наших строк, позволяет удалить либо поменять определенную строку
        int y = 0;
        
        while(true){

            if(save == true){
                printf("=%u=:\r", x); 
            }
            else if(save == false){
                //clear_screen();
                printf("*%u*:", y);
            }

            char command[30] = {0};
            parser_stroke(command, &x, &y); //парсим строку
            x--;
            printf("\033[1A");
            printf("\t\t\t\t\t\t (%u;%u)\n", x, y);
            if(!strcmp(command, "exit")) exit(0);
            parser_words(command); //проверяем и подставляем значения
            y++;
            //parser
        }
    }
}

