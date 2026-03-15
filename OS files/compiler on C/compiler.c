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
#include <string.h> //для работы со строками
#include <unistd.h> //для union
#include <stdint.h> //для регистров (uint8_t и т.д.)
#include <stdbool.h> //для операций над false & true
#include <stdlib.h> //для системных функций
#include <conio.h> //для консоли
#include "assembler.h" //ассемблер
//================================Структуры======================================
bool trace = false; //трассировка

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
}; //список регистров

typedef struct {
    const char* name;     // название команды
    uint8_t opcode;       // код
    bool if_flag;         // есть ли флаги 16/8 bit
    bool if_IR;           // есть ли IR флаг
    uint8_t bytes;        // сколько байт занимает код
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
//================================Функции========================================

void error(char* arg, int y){
    printf("You have an error:%s in %i line, exit from compile", arg, y);
    exit(0);
}

static void load_code_from_input_file(FILE *file, Preprocessing_assembly *assembly){
    char magic_bytes[8]; //+1 для нуль терминатора

    if (!file) {perror("fopen"); exit(1);}
    fread(magic_bytes, 1, 7, file);
    magic_bytes[7] = '\0';
    
    if(strcmp(magic_bytes, "@LET 1 ") == 0){ //если magic bytes совпадают
        assembly->file=file; //в тело ассемблера вставляем исходный файл
        fseek(file, 0, SEEK_SET); //курсрор в файле на начало
        return;
    }
    else{
        error("incorrect magic bytes", -1);
    }
}

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void parser_stroke(char *stroke){ //разбираем строку на слова с разделителем '|'
    for(short i = 0; i<31; i++){
        if(stroke[i] == ' '){
            
        }
    }
}

int parser_words(char *words){
    
}

int main(int argc, char **argv)
{
    clear_screen();
    Preprocessing_assembly assembly;
    FILE *file = fopen("code.swg", "r+");
    load_code_from_input_file(file, &assembly);
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trace") == 0) trace = 1; //включена ли трассировка
    }
    if (trace){printf("Hi, this is LETOS compiler, version %s\n", version);}

    
}

