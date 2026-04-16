#include <stdint.h>
#include <stdio.h>
#define MAX_LINES 200

typedef struct{

    int c_x; //координата начала строки по оси x
    int c_y; //координата строки по оси y
    int y_line; //какая строка
    int symbs; //количество символов
    char* command; //строка через | вместо пробелов

} Line; //структура строк

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
    int count;
    FILE *file; //input file
    Symbol symbols[250]; //колличество меток
    int symbols_count;
    Macros_16_bit macros_16[20];
    Macros_8_bit macros_8[20];
    int macros_count;
} Preprocessing_assembly;

void init_assembler(Preprocessing_assembly* assembly, int count);