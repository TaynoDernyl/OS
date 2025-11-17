#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "compiler.h" 

// ==================== РЕАЛИЗАЦИЯ БАЗОВЫХ ФУНКЦИЙ ====================

void error(char* user_input) {
    printf("Compilation error: %s\n", user_input);
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

bool valid_label(char* label) {
    // Базовая проверка - не пустая строка
    return label != NULL && strlen(label) > 0;
}

bool valid_var(char* variable) {
    // Базовая проверка
    return variable != NULL && strlen(variable) > 0;
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

void add_variable(char* name, uint16_t address, char* type, uint16_t value) {
    if(data_adress_count < 250) {
        strncpy(variables[data_adress_count].name, name, 31);
        strncpy(variables[data_adress_count].type, type, 9);
        variables[data_adress_count].adres = address;
        
        if(strcmp(type, "byte") == 0) {
            variables[data_adress_count].value.byte = (uint8_t)value;
        } else {
            variables[data_adress_count].value.word = value;
        }
        
        data_adress_count++;
        if(trace) printf("Added variable: %s of type %s\n", name, type);
    } else {
        error("Too many variables");
    }
}

int find_label(char* name) {
    for(int i = 0; i < labels_count; i++) {
        if(strcmp(labels[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int find_variable(char* name) {
    for(int i = 0; i < data_adress_count; i++) {
        if(strcmp(variables[i].name, name) == 0) {
            return i;
        }
    }
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

// ==================== РЕАЛИЗАЦИЯ КОМАНД ДЛЯ ВАШЕГО ПРОЦЕССОРА ====================

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

void compile_jmp(char* label) {
    if(trace) printf("JMP %s\n", label);
    
    int label_idx = find_label(label);
    if(label_idx == -1) {
        error("Undefined label in JMP");
        return;
    }
    
    uint16_t address = labels[label_idx].adres;
    
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
}

void second_pass(void) {
    if(trace) printf("=== SECOND PASS ===\n");
    // Заглушка - генерируем тестовый код
    compile_init();
    compile_mov("al", "10");
    compile_mov("ah", "0");
    compile_add("ax", "bx");
    compile_inc("cx");
    compile_setv("px", "py", "al");
    compile_render();
    compile_jmp("loop");
    compile_stop();
}

void reset_compiler(void) {
    labels_count = 0;
    data_adress_count = 0;
    code_adress_count = 0;
    memset(binary_output, 0, sizeof(binary_output));
    if(trace) printf("Compiler reset\n");
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

void print_symbol_table(void) {
    printf("\n=== SYMBOL TABLE ===\n");
    printf("Labels (%d):\n", labels_count);
    for(int i = 0; i < labels_count; i++) {
        printf("  %s: 0x%04X\n", labels[i].name, labels[i].adres);
    }
    
    printf("Variables (%d):\n", data_adress_count);
    for(int i = 0; i < data_adress_count; i++) {
        printf("  %s: 0x%04X (%s)\n", variables[i].name, variables[i].adres, variables[i].type);
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
    if(trace){printf("\n=== BINARY OUTPUT ===\n");
    printf("Code size: %d bytes\n", code_adress_count);
    printf("First 16 bytes dump:\n");
    for(int i = 0; i < code_adress_count && i < 16; i++) {
        printf("%02X ", binary_output[i]);
    }
    printf("\n");}
}

void read_swag(char* path) {
    if(trace){printf("Reading file: %s (stub)\n", path);}
}

void in_bin(char* path) {
    if(trace){printf("Writing binary file: %s (stub)\n", path);}
}

void compile_commands(void) {
    // Заглушка - просто запускаем два прохода
    first_pass();
    second_pass();
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
        printf("=== LETOS COMPILER v0.6 ===\n");
        printf("Trace mode: ON\n");
    }
    
    while (true)
    {
        char line[256] = {0};
        char xD_command[100] = {0};
        char oper1[100] = {0};
        char oper2[100] = {0};
        char oper3[100] = {0};

        printf("> ");
        
        // Читаем всю строку
        if (fgets(line, sizeof(line), stdin)) {
            // Парсим строку
            int args_read = sscanf(line, "%99s %99s %99s %99s", xD_command, oper1, oper2, oper3);
            
            if(trace) {
                printf("[TRACE] Args read: %d\n", args_read);
                printf("[TRACE] Command: '%s', Oper1: '%s', Oper2: '%s', Oper3: '%s'\n", 
                    xD_command, oper1, oper2, oper3);
            }
            
            // Проверяем на NULL (пустые строки)
            if (args_read < 4) oper3[0] = '\0'; // Обнуляем если не ввели
            if (args_read < 3) oper2[0] = '\0';
            if (args_read < 2) oper1[0] = '\0';
            if (args_read < 1) xD_command[0] = '\0';
        }
        
        if(strcmp(xD_command, "exit") == 0){
            if(trace) printf("[TRACE] Exiting compiler...\n");
            break;
        }
        
        if(strcmp(oper1, "=") == 0)
        {
            if(trace) printf("[TRACE] Processing assignment operation\n");
            
            if (find_variable(oper2) == 0)
            {
                if(trace) printf("[TRACE] Source variable '%s' found\n", oper2);
                
                char name[25] = {0};
                char type[10] = {0};
                int number_of_var;
                bool var_id;

                strcpy(name, oper2);
                for(int i = 0; i < sizeof(variables); i++){
                    if (strcmp(name, variables[i].name) == 0){
                        number_of_var = i;
                        strcpy(type, variables[number_of_var].type);
                        
                        if(trace) printf("[TRACE] Found variable at index %d, type: %s\n", i, type);
                        
                        if ((variables[number_of_var].value.word >> 8) == 0){
                            var_id = false;
                            if(trace) printf("[TRACE] Value fits in byte: %u\n", variables[number_of_var].value.word);
                        }
                        else{
                            var_id = true;
                            if(trace) printf("[TRACE] Value requires word: %u\n", variables[number_of_var].value.word);
                        }
                    }
                }
                
                if (var_id){
                    add_variable(xD_command, data_adress_count++, type, variables[number_of_var].value.word);
                    if(trace) printf("[TRACE] Created word variable '%s' with value %u\n", 
                           xD_command, variables[number_of_var].value.word);
                }
                else{
                    add_variable(xD_command, data_adress_count++, type, variables[number_of_var].value.byte);
                    if(trace) printf("[TRACE] Created byte variable '%s' with value %u\n", 
                           xD_command, variables[number_of_var].value.byte);
                }
            }
            else
            {
                if(trace) printf("[TRACE] Source variable '%s' not found\n", oper2);
            }
        }
        
        if(trace) printf("[TRACE] Data address count: %d, Labels count: %d\n", 
               data_adress_count, labels_count);
    }
    
    if(trace) printf("=== Compilation finished ===\n");
    return 0;
}