#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> 
#define MAX_REG 10
//================АРХИТЕКТУРА КОМПИЛЯТОРА==================
bool trace = false;

int labels_count = 0;

int data_adress_count = 0;

int code_adress_count = 0;

const char* proga = NULL;

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
    {"al", 0, 8}, {"ah", 1, 8}, {"bl", 2, 8},  {"bh", 3, 8}, {"ax", 4, 16}, {"bx", 5, 16},  {"ds", 6, 16}, {"cs", 7, 16}, {"cx", 8, 16}, {"px", 9, 8}, {"py", 10, 8}, {NULL, 0, 0} 
};

typedef struct type_table_command {
    char name[10];
    uint8_t code;
    void (*func)(void);   // имя функции как строка
} type_table_command;

type_table_command commands[] = {
    {"NOP", 0x00, },         
    {"load", 0x04, compile_load},
    {"store", 0x05, compile_store},
    {"sub", 0x07, compile_sub},
    {"add", 0x08, compile_add},
    {"inc", 0x09, compile_inc},
    {"dec", 0x0A, compile_dec},
    {"jmp", 0x0B, compile_jmp},
    {"jz", 0x0C, compile_jz},
    {"jnz", 0x0D, compile_jnz},
    {"print", 0x0E, compile_print},
    {"printstr", 0x10, compile_printstr},
    {"input", 0xF9, compile_input},
    {"stop", 0xFF, compile_stop},
    {"setv", 0x30, compile_setv},
    {"render", 0x31, compile_render},
    {"init", 0x32, compile_init},
    {"cmp", 0xD0, compile_cmp},
    {"<", 0xD1, compile_lt},
    {">", 0xD2, compile_gt},
    {"", 0, ""}
};

typedef struct type_label
{
    char name[32];
    uint16_t adres;
} type_label;

type_label labels[100];

typedef struct type_var {
    char name[32];
    uint16_t adres;
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
    uint8_t oper1;
    uint8_t oper2;
    uint8_t oper3;
    void (*function)(void);
}type_command;

type_command commands_for_compile[400];
//========================== ФУНКЦИИ КОМПИЛЯТОРА ================================

//функции компилятора
void add_command_for_compile(char* xD_comamd, int oper1, int oper2, int oper3);
int find_code_command(char* command);
char* find_function_name(uint8_t code);
void compile_load(char* reg, int adres);
void compile_store(int adres, char* reg);
void compile_mov(char* dest, char* src);
void compile_add(char* dest, char* src);
void compile_sub(char* dest, char* src);
void compile_inc(char* reg);
void compile_dec(char* reg);
void compile_jmp(char* label);
void compile_jz(char* label);
void compile_jnz(char* label);
void compile_cmp(char* op1, char* op2);
void compile_lt(uint8_t reg1, uint8_t reg2);
void compile_gt(uint8_t reg1, uint8_t reg2);
void compile_print(void);
void compile_input(void);
void compile_stop(void);
void compile_setv(char* x, char* y, char* color);
void compile_render(void);
void compile_init(void);
void compile_printstr(void);

//функции валидности
bool valid_reg(char* reg);
bool valid_label(char* label);
bool valid_var(char* variable);
int string_to_int(const char* str);

//функции для записи - чтения
void read_swag(char* path);
void in_bin(char* path);

//прерывания ошибки
void error(char* user_input);

//компиляция
void compile_commands(void);

// ========== ДОПОЛНИТЕЛЬНЫЕ ФУНКЦИИ КОМПИЛЯТОРА ==========

// работа с символами
void add_label(char* name, uint16_t address);
void add_variable(char* name, uint16_t address, char* type, uint16_t value);
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