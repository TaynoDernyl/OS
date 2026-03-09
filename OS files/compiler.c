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
    const char* name;     // название команды
    uint8_t opcode;       // код
    bool if_flag;         // есть ли флаги 16/8 bit
    bool if_IR;           // есть ли IR флаг
    uint8_t bytes;
    const char* oswg;     // имя команды в объектном файле
} Command;

Command commands[] = {
    {"NOP",   0x00, false, false, 1, "NOP"},
    {"load",  0x04, false, false, 4, "LOAD"},
    {"store", 0x05, false, false, 4, "STORE"},
    {"sub",   0x07, false, true,  3, "SUB"},
    {"add",   0x08, false, true,  3, "ADD"},
    {"inc",   0x09, false, false, 2, "INC"},
    {"dec",   0x0A, false, false, 2, "DEC"},
    {"jump",  0x0B, false, false, 3, "JUMP"},
    {"jz",    0x0C, false, false, 3, "JZ"},
    {"jnz",   0x0D, false, false, 3, "JNZ"},
    {"print", 0x0E, false, false, 1, "PRINT"},
    {"print_string", 0x10, false, false, 1, "PRINT_STRING"},
    {"mov",   0x20, true,  true,  6, "MOV"},
    {"set_pixel", 0x30, false, false, 4, "SET_PIXEL"},
    {"render",0x31, false, false, 1, "RENDER"},
    {"init",  0x32, false, false, 1, "INIT"},
    {"comp",  0xD0, false, true,  3, "COMP"},
    {"<",     0xD1, false, true,  3, "LT"},
    {">",     0xD2, false, true,  3, "GT"},
    {"input", 0xF9, false, false, 1, "INPUT"},
    {"stop",  0xFF, false, false, 1, "STOP"}
};

typedef struct
{
    char name[32];
    uint16_t adres;
} Symbol;

typedef struct
{
    char name[32]; //имя
    uint8_t meaning; //значение
} Macros_8_bit;

typedef struct
{
    char name[32]; //имя
    uint16_t meaning; //значение
} Macros_16_bit;

typedef struct
{
    Line lines[MAX_LINES]; //максимальное количество строк
    FILE *file; //input file
    Symbol symbols[250]; //колличество меток
    int symbols_count;
    Macros_16_bit macros_16[20];
    Macros_8_bit macros_8[20];
    int macros_count;
} Preprocessing_assembly;

Preprocessing_assembly jertva_aborta;

void init_assembler(Preprocessing_assembly* assembly){
    assembly->macros_count = 0;
    assembly->symbols_count = 0;
    assembly->file = NULL; 
}
bool trace = false; //трассировка
const char* code = NULL; //определение места под нашу программу
bool save = false; //***-если не сохранено, ===-если сохранено

void error(char* arg, int* y){
    printf("You have a error:%s in %u line, exit from compile", arg, *y);
    exit(0);
}



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
        int symb = getch(); 
        *x += 1; //увеличиваем координату х
        
        bool exit = false; //используем чтобы выйти из цикла если true

        switch(symb){

            case '\r':
                putchar('\n');
                exit = true;
                break;

            case 8:
                if(i != 0){
                    i-=2; //убираем индекс последнего символа
                }
                break;

            case 24:
                exit = true;
                strcpy(stroke, "");
                if(trace)printf("[TRACE] Exit & clear buffer\n");
                break;
            
            case 224:
                symb = getch();
                break;
        }

        if (exit == false){
            
            switch (symb)
            {
                case ' ':
                    printf("%c", symb);
                    stroke[i] = '|'; // заменяем пробел на '|'
                    break;

                case '\b': 
                    printf("%c", symb);
                    printf(" ");
                    printf("%c", symb); // стираем последний символ
                    stroke[i + 1] = '\0'; // устанавливаем ноль вместо старого символа
                    break;
                
                case 77:
                    printf("\033[C");
                    break;

                case 75:
                    if (i != 0){
                        printf("\b");
                        i-=2;
                    }
                    break;

                default:
                    if (symb>31 & symb <126){
                        putchar(symb);
                        stroke[i] = symb; //если не подходит под исключения то вставляем символ по индексу в строку command в main функции
                        
                    }
                    else{
                        i--;
                    }
                    break;
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
    clear_screen();
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
            if(!strcmp(command, "exit")) exit(0);
            printf("%s\n", command); //убрать в релизе

            x--;
            y++;

            parser_words(command); //проверяем и подставляем значения
        }
    }
}

