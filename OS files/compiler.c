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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "compiler.h" 
#include <string.h>


// ==================== РЕАЛИЗАЦИЯ БАЗОВЫХ ФУНКЦИЙ ====================

bool word_or_byte(uint16_t number){
    if ((number >> 8) == 0) return 0;
    else return 1;
}

void error(char* user_input) {
    printf("[ERROR]: %s\n", user_input);
    exit(1);
}

bool valid_reg(char* reg) {
    for(int i = 0; registers[i].name != NULL; i++) {
        if(strcmp(registers[i].name, reg) == 0) {
            return true;
        }
    }
    return false;
}

void int_to_string(int number, char* out) {
    char buffer[32];
    int i = 0;
    int sign = number < 0 ? -1 : 1;

    if (number == 0) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    number *= sign;

    while (number > 0) {
        buffer[i++] = (number % 10) + '0';
        number /= 10;
    }

    if (sign < 0) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';

    // Разворачиваем в out
    int len = i;
    for (int j = 0; j < len; j++) {
        out[j] = buffer[len - j - 1];
    }
    out[len] = '\0';

}


bool valid_label(char* label) {
    // Базовая проверка - не пустая строка
    return label != NULL && strlen(label) > 0;
}

bool valid_var(char* variable) {
    // Базовая проверка
    return variable != NULL && strlen(variable) > 0;
}

int find_code_command(char* command){
    for(int i = 0; i < (sizeof(commands))/(sizeof(commands[0])); i++){
        if ((strcmp(command, commands[i].name) == 0)){
            if (trace) printf("[TRACE] Command found! %02X\n", commands[i].code);
            return commands[i].code;
        }
    }
    error("Command not found");
}

void add_label(char* name, uint16_t address) {
    if(labels_count < 100) {
        strncpy(labels[labels_count].name, name, 31);
        labels[labels_count].adres = address;
        labels_count++;
        if(trace) printf("Added label: %s at address %d\n", name, address);
    } else {
        error("Too many labels");
    }
}

void add_variable(char* name, uint16_t address, char* type, uint16_t value, uint8_t count) {
    if (valid_reg(name) == true) error("name is reg");
    if(var_count < 250) {
        strncpy(variables[var_count].name, name, 32);
        strncpy(variables[var_count].type, type, 10);
        variables[var_count].adres = address;
        
        if(strcmp(type, "byte") == 0) {
            variables[var_count].value.byte = (uint8_t)value;
            variables[var_count].count = 1;
        } else if(strcmp(type, "word") == 0){
            variables[var_count].value.word = value;
            variables[var_count].count = 2;
        } else if(strcmp(type, "str") == 0){
            variables[var_count].value.byte = value;
            variables[var_count].count = count;
        } 
        
        data_adress_count += count;
        var_count++;
        if(trace) printf("[TRACE] Added variable: %s of type %s\n", name, type);
    } else {
        error("Too many variables");
    }
}

int delete_variable(char* name){
    short index = find_variable(name);
    if (index >= 0){
    variables[index].adres = -1;
    variables[index].count = -1;
    memset(variables[index].name, 0, sizeof(variables[index].name));
    memset(variables[index].type, 0, sizeof(variables[index].type));
    variables[index].value.word = -1;
    if (trace) printf("[TRACE] Var has deleted: Adres = %d, count = %d, name = %s, type = %s, value = %04d", variables[index].adres, variables[index].count, variables[index].name, variables[index].type, variables[index].value.word);
    data_adress_count -= 1;
    var_count -= 1;
    return index;
    }
    else{
        printf("[TRACE] Var is not found");
        return -1;
    }
}

int find_label(char* name) {
    for(int i = 0; i < labels_count; i++) {
        if(strcmp(variables[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int find_variable(char* name) {
    if (trace) printf("[TRACE] Finding a \'%s\' in: %d\n", name, var_count);
    for(int i = 0; i < var_count; i++) {
        if (trace) printf("[TRACE] Now name is: %s, index = %d\n", variables[i].name, i);
        if(strcmp(variables[i].name, name) == 0) {
            if (trace) printf("[TRACE] Var found at index = %d (name:%s)!\n", i, variables[i].name);
            return i;
        }
    }
    if (trace) printf("[TRACE] Var not found\n");
    return -1;
}

// ==================== КОДЫ РЕГИСТРОВ ====================
uint8_t get_reg_code(char* reg_name) {
    for(int i = 0; registers[i].name != NULL; i++) {
        if(strcmp(registers[i].name, reg_name) == 0) {
            return registers[i].code;
        }
    }
    return 0xFF; // invalid
}

// ==================== РЕАЛИЗАЦИЯ КОМАНД ДЛЯ НАШЕГО ПРОЦЕССОРА ====================

void add_command_for_compile(char* xD_command, char* oper1, char* oper2, char* oper3) {
    if (trace) printf("[TRACE] Add commands for compile: [%s %d %d %d]\n", xD_command, oper1, oper2, oper3);
    
    if (xD_command == NULL || *xD_command == '\0') error("Command is NULL");
    
    uint8_t command_code = find_code_command(xD_command);
    
    // проверяем на специальное значение NO_OPERAND
    if (oper1[0] == NO_OPERAND && oper2[0] == NO_OPERAND && oper3[0] == NO_OPERAND) {
        // команда без операндов
        if(trace) printf("[TRACE] The command has been uploaded: %s (no operands)\n", xD_command);
        strcpy(commands_for_compile[code_compile_count].name, xD_command);
        commands_for_compile[code_compile_count].code = command_code;
        commands_for_compile[code_compile_count].oper1[0] = '\0';
        commands_for_compile[code_compile_count].oper2[0] = '\0';
        commands_for_compile[code_compile_count].oper3[0] = '\0';
        commands_for_compile[code_compile_count].cell = 0;
        code_compile_count++;
    }
    else if (oper2[0] == NO_OPERAND && oper3[0] == NO_OPERAND) {
        // команда с 1 операндом
        if(trace) printf("[TRACE] The command has been uploaded: %s - %s\n", xD_command, oper1);
        strcpy(commands_for_compile[code_compile_count].name, xD_command);
        commands_for_compile[code_compile_count].code = command_code;
        strcpy(commands_for_compile[code_compile_count].oper1, oper1);
        commands_for_compile[code_compile_count].oper2[0] = '\0';
        commands_for_compile[code_compile_count].oper3[0] = '\0';
        commands_for_compile[code_compile_count].cell = 1;
        code_compile_count++;
    }
    else if (oper3[0] == NO_OPERAND) {
        // команда с 2 операндами
        if(trace) printf("[TRACE] The command has been uploaded: %s - %s, %s\n", xD_command, oper1, oper2);
        strcpy(commands_for_compile[code_compile_count].name, xD_command);
        commands_for_compile[code_compile_count].code = command_code;
        strcpy(commands_for_compile[code_compile_count].oper1, oper1);
        strcpy(commands_for_compile[code_compile_count].oper2, oper2);
        commands_for_compile[code_compile_count].oper3[0] = '\0';
        commands_for_compile[code_compile_count].cell = 2;
        code_compile_count++;
    }
    else {
        // команда с 3 операндами
        if(trace) printf("[TRACE] The command has been uploaded: %s - %s, %s, %s\n", xD_command, oper1, oper2, oper3);
        strcpy(commands_for_compile[code_compile_count].name, xD_command);
        commands_for_compile[code_compile_count].code = command_code;
        strcpy(commands_for_compile[code_compile_count].oper1, oper1);
        strcpy(commands_for_compile[code_compile_count].oper2, oper2);
        strcpy(commands_for_compile[code_compile_count].oper3, oper3);
        commands_for_compile[code_compile_count].cell = 3;
        code_compile_count++;
    }
}

void compile_load(char* reg, int address) {
    if(trace) printf("compile_load: %s, %d\n", reg, address);
    
    uint8_t reg_code = get_reg_code(reg);
    
    binary_output[code_adress_count++] = 0x04; // load opcode
    binary_output[code_adress_count++] = reg_code;
    binary_output[code_adress_count++] = address & 0xff;
    binary_output[code_adress_count++] = (address >> 8) & 0xff;
}

void compile_store(int address, char* reg) {
    if(trace) printf("compile_store: %d, %s\n", address, reg);
    
    uint8_t reg_code = get_reg_code(reg);
    
    binary_output[code_adress_count++] = 0x05; // store opcode
    binary_output[code_adress_count++] = reg_code;
    binary_output[code_adress_count++] = address & 0xff;
    binary_output[code_adress_count++] = (address >> 8) & 0xff;
}

void compile_lt(char* reg1, char* reg2) {
    if(trace) printf("compile_lt: %s, %s\n", reg1, reg2);
    
    uint8_t reg1_code = get_reg_code(reg1);
    uint8_t reg2_code = get_reg_code(reg2);
    
    binary_output[code_adress_count++] = 0xd1; // < opcode
    binary_output[code_adress_count++] = reg1_code;
    binary_output[code_adress_count++] = reg2_code;
}

void compile_gt(char* reg1, char* reg2) {
    if(trace) printf("compile_gt: %s, %s\n", reg1, reg2);
    
    uint8_t reg1_code = get_reg_code(reg1);
    uint8_t reg2_code = get_reg_code(reg2);
    
    binary_output[code_adress_count++] = 0xd2; // > opcode
    binary_output[code_adress_count++] = reg1_code;
    binary_output[code_adress_count++] = reg2_code;
}

void compile_mov(char* dest, char* src) {
    if(trace) printf("MOV %s, %s\n", dest, src);
    
    uint8_t dest_code = get_reg_code(dest);
    uint8_t src_code = get_reg_code(src);
    
    // Определяем размер операндов
    uint8_t flags = 0;
    if (dest_code <= MAX_REG) { // 8-bit регистры
        flags = 0;
        binary_output[code_adress_count++] = 0x20; // MOV opcode
        binary_output[code_adress_count++] = flags;
        binary_output[code_adress_count++] = dest_code;
        binary_output[code_adress_count++] = src_code;
    } else { // 16-bit регистры
        flags = 1;
        binary_output[code_adress_count++] = 0x20; // MOV opcode
        binary_output[code_adress_count++] = flags;
        binary_output[code_adress_count++] = dest_code;
        binary_output[code_adress_count++] = src_code;
        // Для 16-bit MOV может потребоваться дополнительный байт
        binary_output[code_adress_count++] = 0x00; // placeholder
    }
}

void compile_add(char* dest, char* src) {
    if(trace) printf("ADD %s, %s\n", dest, src);
    
    uint8_t dest_code = get_reg_code(dest);
    uint8_t src_code = get_reg_code(src);
    
    binary_output[code_adress_count++] = 0x08; // ADD opcode
    binary_output[code_adress_count++] = dest_code;
    binary_output[code_adress_count++] = src_code;
}

void compile_sub(char* dest, char* src) {
    if(trace) printf("SUB %s, %s\n", dest, src);
    
    uint8_t dest_code = get_reg_code(dest);
    uint8_t src_code = get_reg_code(src);
    
    binary_output[code_adress_count++] = 0x07; // SUB opcode
    binary_output[code_adress_count++] = dest_code;
    binary_output[code_adress_count++] = src_code;
}

void compile_inc(char* reg) {
    if(trace) printf("INC %s\n", reg);
    
    uint8_t reg_code = get_reg_code(reg);
    
    binary_output[code_adress_count++] = 0x09; // INC opcode
    binary_output[code_adress_count++] = reg_code;
}

void compile_dec(char* reg) {
    if(trace) printf("DEC %s\n", reg);
    
    uint8_t reg_code = get_reg_code(reg);
    
    binary_output[code_adress_count++] = 0x0A; // DEC opcode
    binary_output[code_adress_count++] = reg_code;
}

void compile_jmp(int address) {
    if(trace) printf("[TRACE] JMP %d\n", address);
    address = (uint16_t)address;
    binary_output[code_adress_count++] = 0x0B; // JMP opcode
    binary_output[code_adress_count++] = address & 0xFF;        // low byte
    binary_output[code_adress_count++] = (address >> 8) & 0xFF; // high byte
}

void compile_jz(char* label) {
    if(trace) printf("JZ %s\n", label);
    
    int label_idx = find_label(label);
    if(label_idx == -1) {
        error("Undefined label in JZ");
        return;
    }
    
    uint16_t address = labels[label_idx].adres;
    
    binary_output[code_adress_count++] = 0x0C; // JZ opcode
    binary_output[code_adress_count++] = address & 0xFF;        // low byte
    binary_output[code_adress_count++] = (address >> 8) & 0xFF; // high byte
}

void compile_jnz(char* label) {
    if(trace) printf("JNZ %s\n", label);
    
    int label_idx = find_label(label);
    if(label_idx == -1) {
        error("Undefined label in JNZ");
        return;
    }
    
    uint16_t address = labels[label_idx].adres;
    
    binary_output[code_adress_count++] = 0x0D; // JNZ opcode
    binary_output[code_adress_count++] = address & 0xFF;        // low byte
    binary_output[code_adress_count++] = (address >> 8) & 0xFF; // high byte
}

void compile_cmp(char* op1, char* op2) {
    if(trace) printf("CMP %s, %s\n", op1, op2);
    
    uint8_t op1_code = get_reg_code(op1);
    uint8_t op2_code = get_reg_code(op2);
    
    binary_output[code_adress_count++] = 0xD0; // CMP (equal) opcode
    binary_output[code_adress_count++] = op1_code;
    binary_output[code_adress_count++] = op2_code;
}

void compile_print(void) {
    if(trace) printf("PRINT\n");
    binary_output[code_adress_count++] = 0x0E; // PRINT char opcode
}

void compile_input(void) {
    if(trace) printf("INPUT\n");
    binary_output[code_adress_count++] = 0xF9; // INPUT opcode
}

void compile_stop(void) {
    if(trace) printf("STOP\n");
    binary_output[code_adress_count++] = 0xFF; // HALT opcode
}

void compile_setv(char* x, char* y, char* color) {
    if(trace) printf("SETV %s, %s, %s\n", x, y, color);

    uint8_t x_code = get_reg_code(x);
    uint8_t y_code = get_reg_code(y);
    uint8_t color_code = get_reg_code(color);
    
    binary_output[code_adress_count++] = 0x30; // SETV opcode
    binary_output[code_adress_count++] = x_code;
    binary_output[code_adress_count++] = y_code;
    binary_output[code_adress_count++] = color_code;
}

void compile_render(void) {
    if(trace) printf("RENDER\n");
    binary_output[code_adress_count++] = 0x31; // RENDER opcode
}

void compile_init(void) {
    if(trace) printf("INIT\n");
    binary_output[code_adress_count++] = 0x32; // INIT opcode
}

void compile_printstr(void) {
    if(trace) printf("PRINTSTR\n");
    binary_output[code_adress_count++] = 0x10; // PRINT_STR opcode
}

// ==================== ПАРСИНГ И КОМПИЛЯЦИЯ ====================

void parse_line(char* line) {
    // Заглушка для парсинга - просто выводим строку
    if(trace) printf("Parsing: %s\n", line);
}

void first_pass(void) {
    if(trace) printf("=== FIRST PASS ===\n");
    for(int i = 0; i < sizeof(commands_for_compile)/sizeof(commands_for_compile[0]); i++){ //меняем метки на адреса, переменные на значения либо адреса строк
        
        if (commands_for_compile[i].name[0] == '\0') continue;
        int index_label = find_label(commands_for_compile[i].oper1);
        int index_var = find_variable(commands_for_compile[i].oper1);
        if(trace) printf("[TRACE] INDEX var:%d, INDEX label:%d\n", index_var, index_label);

        if (index_var >= 0) {
            char string[32];
            int_to_string((int)variables[index_var].value.word, string);
            strcpy(commands_for_compile[i].oper1, string);
            if (trace) printf("[TRACE] oper1 (i = %d) var is: %s\n",i, commands_for_compile[i].oper1);
        }
    }
}

void second_pass(void) {
    if(trace) printf("=== SECOND PASS ===\n");
    
    // Используйте code_compile_count вместо sizeof
    for(int i = 0; i < code_compile_count; i++){
        if (trace) printf("[TRACE] %d - number of command\n", i);
        
        char* name = commands_for_compile[i].name;
        if (trace) printf("[TRACE] Now command is: %s\n", name);
        
        // Проверка на пустую команду
        if(name == NULL || strlen(name) == 0) {
            if(trace) printf("[TRACE] Skipping empty command\n");
            continue;
        }
        
        for(int j = 0; j < (sizeof(commands))/(sizeof(commands[0])); j++){
            if (strcmp(commands[j].name, name) == 0){
                if (trace) printf("[TRACE] Found name: %s\n", commands[j].name);
                if(commands[j].function != NULL) {
                    commands[j].function();
                } else {
                    if(trace) printf("[TRACE] Function pointer is NULL\n");
                }
                break;
            }
        }
    }
}
void reset_compiler(void) {
    labels_count = 0;
    data_adress_count = 0;
    code_adress_count = 0;
    memset(binary_output, 0, sizeof(binary_output));
    if(trace) printf("Compiler reset\n");
}

void compile(void){
    first_pass();
    second_pass();
}

uint16_t parse_number(char* str) {
    return (uint16_t)atoi(str);
}

uint16_t resolve_address(char* name) {
    int idx = find_label(name);
    if(idx >= 0) return labels[idx].adres;
    
    idx = find_variable(name);
    if(idx >= 0) return variables[idx].adres;
    
    return 0xFFFF; // не найдено
}

// ==================== УТИЛИТЫ ====================

// Функция для обработки присваивания существующей переменной
void handle_existing_variable_assignment(char* var_name, char* oper2, int index) {
    if(trace) printf("[TRACE] Source variable '%s' found\n", var_name);
    
    char type[10] = {0};
    strcpy(type, variables[index].type);
    char var_id[6];
    
    union {
        uint8_t byte;
        uint16_t word;
    } old_value;
    old_value.word = variables[index].value.word;
    int adres = variables[index].adres;
    uint8_t old_count = variables[index].count;
    union {
        uint8_t byte;
        uint16_t word;
    } value;
    
    // Определяем тип переменной
    if (strcmp(type, "byte") == 0 || strcmp(type, "word") == 0) {
        if ((variables[index].value.word >> 8) == 0) {
            strcpy(var_id, "byte");
            if(trace) printf("[TRACE] Value fits in byte: %u\n", old_value.byte);
        }
        else {
            strcpy(var_id, "word");
            if(trace) printf("[TRACE] Value requires word: %u\n", old_value.word);
        }
        value.word = string_to_int(oper2);
    }
    else if (strcmp(type, "str") == 0) {
        strcpy(var_id, "str");
        int count;
        for(int i = 0; ; i++) {
            if (oper2[i] == 0) break;
            data_adress_count++;
            count++;
        }
        if(trace) printf("[TRACE] Value is str: first char '%c'\n", (char)old_value.byte);
        count -= 1;
        if (old_count < count) error("String is so long!");
        if (oper2[0] == '\"') value.byte = oper2[1];
        else value.byte = oper2[0];
    }
    else {
        error("Unknown variable type");
    }
    
    // Обработка присваивания значения
    if (oper2[0] == '\"' || (oper2[0] >= '0' && oper2[0] <= '9')) {
        delete_variable(var_name);
        if (trace) printf("[TRACE] Stroke: %s\n", oper2);
        
        if (strcmp(var_id, "word") == 0) {
            add_variable(var_name, adres, type, value.word, old_count);
            if(trace) printf("[TRACE] Created word variable '%s' with value %u\n", 
                var_name, value.word);
        }
        else if (strcmp(var_id, "byte") == 0) {
            add_variable(var_name, adres, type, value.byte, old_count);
            if(trace) printf("[TRACE] Created byte variable '%s' with value %u\n",
                var_name, value.byte);
        }
        else if (strcmp(var_id, "str") == 0) {
            add_variable(var_name, adres, "str", value.byte, old_count);
            if(trace) printf("[TRACE] Created str variable '%s' with adres %u\n", 
                var_name, adres);
        }
        else error("No type var");
    }
    else {
        if (trace) printf("[TRACE] Var: %s\n", oper2);
        if (find_variable(oper2) >= 0) {
            if (strcmp(var_id, "word") == 0) {
                add_variable(var_name, adres, type, old_value.word, old_count);
                if(trace) printf("[TRACE] Created word variable '%s' with value %u\n", 
                    var_name, old_value.word);
            }
            else if (strcmp(var_id, "byte") == 0) {
                add_variable(var_name, adres, type, old_value.byte, old_count);
                if(trace) printf("[TRACE] Created byte variable '%s' with value %u\n",
                    var_name, string_to_int(oper2));
            }
            else if (strcmp(var_id, "str") == 0) {
                add_variable(var_name, adres, "str", old_value.byte, old_count);
                if(trace) printf("[TRACE] Created str variable '%s' with adres %u\n", 
                    var_name, adres);
            }
            else error("No type var");
        }
        else {
            error("Var is not declared");
        }
    }
}

// функция для создания новой переменной
void create_new_variable(char* var_name, char* oper2) {
    if ((*var_name >= '0' && *var_name <= '9') != true) {
        if (find_variable(oper2) == -1) {
            if (oper2[1] >= '@' && (oper2[1] <= 'Z' || oper2[1] >= ' ') && oper2[0] == '\"') {
                int count = 0;
                for(int i = 0; ; i++) {
                    if (oper2[i] == 0) break;
                    count++;
                }
                if (oper2[0] == '\"'){
                    add_variable(var_name, data_adress_count, "str", oper2[1], count+1);
                    data_adress_count -= 1;}
                
                int index = find_variable(var_name);
                variables[index].count = count;
                if(trace) printf("[TRACE] Source variable was created, name is: \"%s\", string is: \"%s\", adres: %d\n", 
                    var_name, oper2, data_adress_count - count - 1);
            }
            else {
                if (oper2[0] >= '@' && (oper2[0] <= '/' || oper2[0] >= ' ')) 
                    error("Not found \"");
                
                int value = string_to_int(oper2);
                int index = var_count; // Текущий индекс перед добавлением
                
                if (value > 255) {
                    add_variable(var_name, data_adress_count, "word", value, 2); 
                }
                else {
                    add_variable(var_name, data_adress_count, "byte", value, 1); 
                }
                if(trace) printf("[TRACE] Source variable was created, name is: \"%s\", value is: \"%d\", adres: %d\n", 
                    var_name, value, data_adress_count - 1);
            }
        }
        else {
            int index = find_variable(oper2);
            char type[10] = {0};
            strcpy(type, variables[index].type);
            char var_id[6];
            
            union {
                uint8_t byte;
                uint16_t word;
            } old_value;
            old_value.word = variables[index].value.word;
            int adres = data_adress_count;
            uint8_t old_count = variables[index].count;
            if (strcmp(type, "byte") == 0 || strcmp(type, "word") == 0) {
                if ((variables[index].value.word >> 8) == 0) {
                    strcpy(var_id, "byte");
                }
                else {
                    strcpy(var_id, "word");
                }
            }
            else if (strcmp(type, "str") == 0) {
                strcpy(var_id, "str");
            }
            else {
                error("Unknown variable type");
            }
            
            if (strcmp(var_id, "word") == 0) {
                add_variable(var_name, adres, type, old_value.word, old_count);
                if(trace) printf("[TRACE] Created word variable '%s' with value %u\n", 
                    var_name, old_value.word);
            }
            else if (strcmp(var_id, "byte") == 0) {
                add_variable(var_name, adres, type, old_value.byte, old_count);
                if(trace) printf("[TRACE] Created byte variable '%s' with value %u\n",
                    var_name, old_value.byte);
            }
            else if (strcmp(var_id, "str") == 0) {
                add_variable(var_name, adres, "str", old_value.byte, old_count);
                if(trace) printf("[TRACE] Created str variable '%s' with adres %u\n", 
                    var_name, adres);
            }
            else error("No type var");
        }
        print_symbol_table();
    }
    else {
        error("The name of the variable begins with a number");
    }
}

void print_symbol_table(void) {
    printf("\n=== SYMBOL TABLE ===\n");
    printf("Labels (%d):\n", labels_count);
    for(int i = 0; i < labels_count; i++) {
        printf("  %s: 0x%04X\n", labels[i].name, labels[i].adres);
    }
    
    printf("Variables (%d):\n", var_count);
    for(int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].type, "str") == 0) {
            char temp = (char)variables[i].value.byte;
            printf("  %s: 0x%04X (%s) = %c, size = %d\n", variables[i].name, variables[i].adres, variables[i].type, temp, variables[i].count);
        }
        else {
            if (strcmp(variables[i].type, "word") == 0) printf("  %s: 0x%04X (%s) = %04d, size = %d\n", variables[i].name, variables[i].adres, variables[i].type, variables[i].value.word, variables[i].count);
            else printf("  %s: 0x%04X (%s) = %03d, size = %d\n", variables[i].name, variables[i].adres, variables[i].type, variables[i].value.byte, variables[i].count);
        }
    }
}

int string_to_int(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;
    
    // Обработка знака
    if (str[0] == '-') {
        sign = -1;
        i++;
    } else if (str[0] == '+') {
        i++;
    }
    
    // Преобразование цифр
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    
    return result * sign;
}

void print_binary_info(void) {
    if(trace){
        printf("\n=== BINARY OUTPUT ===\n");
        printf("Code size: %d bytes\n", code_adress_count);
        printf("First 16 bytes dump:\n");
        for(int i = 0; i < code_adress_count && i < 16; i++) {
            printf("%02X ", binary_output[i]);
        }
        printf("\n");
    }
}

void read_swag(char* path) {
    if(trace){printf("Reading file: %s (stub)\n", path);}
}

void in_bin(char* path) {
    if(trace){printf("Writing binary file: %s (stub)\n", path);}
}

void compile_commands(void) {
    first_pass();
    second_pass();
}

// ==================== TEST ====================

void test(){
    
}

// ==================== MAIN ====================

int main(int argc, char **argv) {
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-t") == 0){ // если в аргументах есть -t то включаем трассировку, если нет то устанавливаем имя компилируемой программы
            trace = true;
        }
        else{
            proga = argv[i];
        }
    }

    if(trace){
        printf("=== LETOS COMPILER v0.0.6 ===\n");
        printf("Trace mode: ON\n");
    }

    while (true)
    {
        char line[256] = {0};
        char xD_command[100] = {0};
        char oper1[32] = {NO_OPERAND};
        char oper2[32] = {NO_OPERAND};
        char oper3[32] = {NO_OPERAND};

        printf("> ");
        
        // читаем всю строку
        if (fgets(line, sizeof(line), stdin)) {
            // парсим строку
            int args_read = sscanf(line, "%99s %99s %99s %99s", xD_command, oper1, oper2, oper3);
            if (((oper1[0] >= '0' || oper1[0] <= '9') || (oper2[0] >= '0' || oper2[0] <= '9') || (oper3[0] >= '0' || oper3[0] <= '9')) != 1){
                for(int i = 0; i < ((sizeof(variables))/(sizeof(variables[0]))); i++){
                    char* name = variables[i].name;
                    if (strcmp(name, oper1) == 1) {memset(oper1, 0, sizeof(oper1));oper1[0] = variables[i].value.word; printf("[TRACE] Oper1 var is: %d", oper1[0]);}
                    if (strcmp(name, oper2) == 1) {memset(oper2, 0, sizeof(oper2));oper2[0] = variables[i].value.word; printf("[TRACE] Oper2 var is: %d", oper1[0]);}
                    if (strcmp(name, oper3) == 1) {memset(oper3, 0, sizeof(oper3));oper3[0] = variables[i].value.word; printf("[TRACE] Oper3 var is: %d", oper1[0]);}
                }
            }
            if(trace) {
                printf("[TRACE] Args read: %d\n", args_read);
                printf("[TRACE] Command: '%s', Oper1: '%s', Oper2: '%s', Oper3: '%s'\n", 
                    xD_command, oper1, oper2, oper3);
            }
            
            // проверяем на NULL (пустые строки)
            if (args_read < 4) oper3[0] = NO_OPERAND; // обнуляем если не ввели
            if (args_read < 3) oper2[0] = NO_OPERAND;
            if (args_read < 2) oper1[0] = NO_OPERAND;
            if (args_read < 1) xD_command[0] = NO_OPERAND;
        }
        
        if(strcmp(xD_command, "exit") == 0){
            if(trace) printf("[TRACE] Exiting compiler...\n");
            break;
        }
        
        if(strcmp(oper1, "=") == 0) { // если создаем переменную
            if(trace) printf("[TRACE] Processing assignment operation\n");
            
            short index = find_variable(xD_command);
            if (trace) printf("[TRACE] INDEX IS: %d \n", index);
            
            if (index >= 0) { // переменная уже существует
                handle_existing_variable_assignment(xD_command, oper2, index);
            }
            else { // создаем новую переменную
                create_new_variable(xD_command, oper2);
            }
        }
        else{
            add_command_for_compile(xD_command, oper1, oper2, oper3);
            compile();   
        }
        
        if(trace) printf("[TRACE] Data address count: %d, Labels count: %d\n", 
               data_adress_count, labels_count);  
    }
    
    if(trace) printf("=== Compilation finished ===\n");
    return 0;
}