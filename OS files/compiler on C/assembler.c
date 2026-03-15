#include "assembler.h"

void init_assembler(Preprocessing_assembly* assembly){
    assembly->macros_count = 0;
    assembly->symbols_count = 0;
    assembly->file = NULL; 
}

