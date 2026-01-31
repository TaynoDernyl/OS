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
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> 
#define MAX_REG 10
#define NO_OPERAND -1
//================АРХИТЕКТУРА КОМПИЛЯТОРА==================
bool trace = false;

bool checking_the_numbers = 1;

int code_compiling_count = 0; //для компиляции, счетчик сколько комманд ввел пользователь

int count_checking_the_numbers = 0;

int labels_count = 0;

int data_adress_count = 0;

int code_adress_count = 0; //сколько команд будет скомпилировано

int var_count = 0;

char* proga = NULL;

int swag_size = 0;

int code_compile_count = 0;

uint8_t binary_output[65536];

typedef struct type_reg
{
    char* name;
    uint8_t code;
    uint8_t bitness;
} type_reg;

type_reg registers[] = {
    {"al", 0, 8}, {"ah", 1, 8}, {"bl", 2, 8},  {"bh", 3, 8}, {"ax", 4, 16}, {"bx", 5, 16},  {"ds", 6, 16}, {"cs", 7, 16}, {"cx", 8, 8}, {"px", 9, 8}, {"py", 10, 8}, {NULL, 0, 0} 
};

// объявляем типы функций
typedef void (*func_no_args)(void);
typedef void (*func_one_arg)(char*);
typedef void (*func_two_args)(char*, char*);
typedef void (*func_three_args)(char*, char*, char*);

typedef struct type_label
{
    char name[32];
    uint16_t adres;
} type_label;

type_label labels[100];

typedef struct type_var {
    char name[32];
    uint16_t adres;
    uint8_t count;
    char type[10];
    union {
        uint8_t byte;
        uint16_t word;
    }value;
} type_var;

type_var variables[250];

typedef struct type_command
{
    char name[10];
    uint8_t code;
    char oper1[32];
    char oper2[32];
    char oper3[32];
    uint8_t cell;
    void (*function)(void);
}type_command;

type_command commands_for_compile[400];
//========================== ФУНКЦИИ КОМПИЛЯТОРА ================================

//функции компилятора
void add_command_for_compile(char* xD_comamd, char* oper1, char* oper2, char* oper3);
uint8_t find_code_command(char* command);
char* find_function_name(uint8_t code);
void handle_existing_variable_assignment(char* var_name, char* oper2, int index);
void create_new_variable(char* var_name, char* oper2);
void compile_load(int reg_code, int address);
void compile_store(int address, int reg_code);
void compile_mov(int dest_code, int src_code);
void compile_add(int dest_code, int src_code);
void compile_sub(int dest_code, int src_code);
void compile_inc(int reg_code);
void compile_dec(int reg_code);
void compile_jmp(int address);
void compile_jz(int label_address);
void compile_jnz(int label_address);
void compile_cmp(int op1_code, int op2_code);
void compile_lt(int reg1_code, int reg2_code);
void compile_gt(int reg1_code, int reg2_code);
void compile_print(void);
void compile_input(void);
void compile_stop(void);
void compile_setv(int x_code, int y_code, int color_code);
void compile_render(void);
void compile_init(void);
void compile_printstr(void);
void compile_stop(void);

typedef struct type_table_command {
    char name[10];
    uint8_t code;
    uint8_t args_count;  // количество аргументов
    void (*function)();   // универсальный указатель
} type_table_command;

type_table_command commands[] = {
    {"nop", 0x00, 0, NULL},         
    {"load", 0x04, 2, compile_load},
    {"store", 0x05, 2, compile_store},
    {"sub", 0x07, 2, compile_sub},
    {"add", 0x08, 2, compile_add},
    {"inc", 0x09, 1, compile_inc},
    {"dec", 0x0a, 1, compile_dec},
    {"jmp", 0x0b, 2, compile_jmp},
    {"jz", 0x0c, 2, compile_jz},
    {"jnz", 0x0d, 2, compile_jnz},
    {"print", 0x0e, 0, compile_print},
    {"printstr", 0x10, 0, compile_printstr},
    {"mov", 0x20, 3, compile_mov},
    {"input", 0xf9, 0, compile_input},
    {"stop", 0xff, 0, compile_stop},
    {"setv", 0x30, 3, compile_setv},
    {"render", 0x31, 0, compile_render},
    {"init", 0x32, 0, compile_init},
    {"cmp", 0xd0, 2, compile_cmp},
    {"<", 0xd1, 2, compile_lt},
    {">", 0xd2, 2, compile_gt},
    {"stop", 0xff, 0, compile_stop},
    {"", 0, 0, NULL}
};

typedef struct
{
    char name[20];
    void (*function)(void);
} systemcommand;
// Системные команды компилятора
void print_binary_info(void);     // bin - вывод бинарной информации
void reset_compiler(void);        // reset - сброс состояния компилятора
void print_symbol_table(void);    // vars/labels - вывод таблицы символов
void compile(void);
systemcommand systemcommands[] =
{
    {"bin", (void(*)(void))print_binary_info},
    {"reset", (void(*)(void))reset_compiler},
    {"vars", (void(*)(void))print_symbol_table},
    {"labels", (void(*)(void))print_symbol_table},
    {"compile", (void(*)(void))compile},
    {"", NULL}  // Терминатор массива
};

//функции валидности
int if_system_command(char*);
bool valid_reg(char* reg);
bool valid_label(char* label);
bool valid_var(char* variable);
int string_to_int(const char* str);
void int_to_string(int, char*);
//функции для записи - чтения
void read_swag(char* path);
void in_bin(char* name_file);


//прерывания ошибки
void error(char* user_input);

//компиляция
void compile_commands(void);

// ========== ДОПОЛНИТЕЛЬНЫЕ ФУНКЦИИ КОМПИЛЯТОРА ==========

// работа с символами
void add_label(char* name, uint16_t address);
void add_variable(char* name, uint16_t address, char* type, uint16_t value, uint8_t count);
int delete_variable(char* name);
int delete_label(char* name);
int find_label(char* name);
int find_variable(char* name);

// парсинг
void parse_line(char* line);

// управление компиляцией
void first_pass(void);    // Первый проход - сбор меток
void second_pass(void);   // Второй проход - генерация кода
void reset_compiler(void); // Сброс компилятора

// утилиты
uint16_t parse_number(char* str);
uint16_t resolve_address(char* name);
void print_symbol_table(void);
void print_binary_info(void);
