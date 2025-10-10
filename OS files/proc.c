#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MEM_SIZE 65536   // <- исправлено: 2^16
#define MOV 0x20
#define NOP 0x00
#define LOAD_A(x) 0x01, (uint8_t)(x) //загрузить сразу в регистр A
#define LOAD_B(x) 0x02, (uint8_t)(x) //загрузить сращу в регистр В
#define LOADMEM_A(addr) 0x03, (uint16_t)((addr) & 0xFF), (uint16_t)(((addr) >> 8) & 0xFF)
#define LOADMEM_B(addr) 0x04, (uint16_t)((addr) & 0xFF), (uint16_t)(((addr) >> 8 )& 0xFF)
#define STA(addr) 0x05, (uint16_t)((addr) & 0xFF), (uint16_t)(((addr) >> 8) & 0xFF)
#define STB(addr) 0x06, (uint16_t)((addr) & 0xFF), (uint16_t)(((addr) >> 8) & 0xFF)
#define SUB 0x07
#define ADD 0x08
#define INC 0x09
#define DEC 0x0A
#define JMP(addr) 0x0B, (uint8_t)((addr) & 0xFF), (uint8_t)(((addr) >> 8) & 0xFF)
#define JZ(addr) 0x0C, (uint8_t)((addr) & 0xFF),  (uint8_t)(((addr) >> 8) & 0xFF)
#define JNZ(addr) 0x0D, (uint8_t)((addr) & 0xFF),  (uint8_t)(((addr) >> 8) & 0xFF)
#define PRINTAL 0x0E
#define PRINTA 0xAE
#define HLT 0xFF
#define LDAL(x) 0xA1, (uint8_t)(x)
#define LDAH(x) 0xA2, (uint8_t)(x)
#define LDBL(x) 0xB1, (uint8_t)(x)
#define LDBH(x) 0xB2, (uint8_t)(x)

enum {
    REG_AL = 0,
    REG_AH = 1,
    REG_BL = 2,
    REG_BH = 3,
    REG_AX = 4,
    REG_BX = 5,
};

typedef struct 
{
    uint8_t AL, AH, BL, BH;
    uint16_t AX, BX;
    uint16_t PC;
    uint8_t Z;
} CPU;

static uint8_t mem[MEM_SIZE];

static uint16_t rd16(uint16_t PC){ //читаем 16 битное число (безопасно по модулю MEM_SIZE)
    uint16_t lo = mem[PC % MEM_SIZE];
    uint16_t hi = mem[(PC+1) % MEM_SIZE];
    return (hi << 8) | lo;
}

static void setZ(CPU *cpu, uint16_t v) { cpu->Z = (v == 0); }

static void load_binary(const char *path){
    FILE *f = fopen(path, "rb");
    if (!f) {perror("fopen"); exit(1);}
    size_t n = fread(mem, 1, MEM_SIZE, f);  
    fclose(f);
    if (n == 0){fprintf(stderr, "empty\n"); exit(1);}
}

static void load_demo_program(void){
    uint8_t demo[] = {MOV,0,0,33,0xD0,0,33,0x0E, 0xFF};
    memset(mem, 0, MEM_SIZE);
    memcpy(mem, demo, sizeof(demo));
}

int main(int argc, char **argv) {
    printf("Welcome to Letos 0.0.3!\n");
    int trace = 0;
    const char *prog = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trace") == 0) trace = 1;
        else prog = argv[i];
    }

    if (prog) load_binary(prog); else load_demo_program();

    CPU cpu = {.PC = 0, .Z = 0, .AL = 0, .AH = 0, .BL =0, .BH = 0, .AX = 0, .BX = 0};

    for (;;) {
        uint8_t op = mem[cpu.PC++ % MEM_SIZE];
        if (trace) {
            printf("\n PC=%04u OP=%02X Z=%02d AL=%02u AH=%02u BL=%02u BH=%02u AX=%04u BX=%04u\n",
                (cpu.PC-1) & 0xFFFF, op, cpu.Z, cpu.AL, cpu.AH, cpu.BL, cpu.BH, cpu.AX, cpu.BX);
        }
        
        switch (op) {
            case MOV: { 
                uint8_t flags = mem[cpu.PC++ % MEM_SIZE];
                uint8_t dst = mem[cpu.PC++ % MEM_SIZE];
                uint8_t src = mem[cpu.PC++ % MEM_SIZE];

                // debug: вывести считанные аргументы (удалить, если не нужно)
                if (trace) printf("MOV flags=%u dst=%u src=%u\n", flags, dst, src);
                
                if (flags == 0) { // MOV 8-бит
                    uint8_t val = 0;
                    switch (src) {
                        case REG_AL: val = cpu.AL; break;
                        case REG_AH: val = cpu.AH; break;
                        case REG_BL: val = cpu.BL; break;
                        case REG_BH: val = cpu.BH; break;
                        default:
                            val = src;
                            break; // пропускаем, не выходим из программы
                    }
                    switch (dst) {
                        case REG_AL: cpu.AL = val; break;
                        case REG_AH: cpu.AH = val; break;
                        case REG_BL: cpu.BL = val; break;
                        case REG_BH: cpu.BH = val; break;
                        default:
                            fprintf(stderr, "Invalid 8-bit dst reg %u (flags=%u,src=%u)\n", dst, flags, src);
                            break;
                    }
                } else { // MOV 16-бит
                    uint16_t val = 0;
                    switch (src) {
                        case REG_AX: val = cpu.AX; break;
                        case REG_BX: val = cpu.BX; break;
                        default:
                            val = src;
                            break;
                    }
                    switch (dst) {
                        case REG_AX: cpu.AX = val; break;
                        case REG_BX: cpu.BX = val; break;
                        default:
                            fprintf(stderr, "Invalid 16-bit dst reg %u (flags=%u,src=%u)\n", dst, flags, src);
                            break;
                    }
                }
            } break;

            case 0x00: /* NOP */ break;

            case 0x04: { // LOAD регистр, [addr16]
                uint8_t reg = mem[cpu.PC++ % MEM_SIZE];   // какой регистр загружаем
                uint16_t addr = rd16(cpu.PC); 
                cpu.PC += 2;

                switch (reg) {
                    case REG_AL:
                        cpu.AL = mem[addr % MEM_SIZE];
                        setZ(&cpu, cpu.AL);
                        break;
                    case REG_AH:
                        cpu.AH = mem[addr % MEM_SIZE];
                        setZ(&cpu, cpu.AH);
                        break;
                    case REG_BL:
                        cpu.BL = mem[addr % MEM_SIZE];
                        setZ(&cpu, cpu.BL);
                        break;
                    case REG_BH:
                        cpu.BH = mem[addr % MEM_SIZE];
                        setZ(&cpu, cpu.BH);
                        break;
                    case REG_AX: {
                        uint8_t lo = mem[addr % MEM_SIZE];
                        uint8_t hi = mem[(addr + 1) % MEM_SIZE];
                        cpu.AX = (hi << 8) | lo;
                        setZ(&cpu, cpu.AX);
                    } break;
                    case REG_BX: {
                        uint8_t lo = mem[addr % MEM_SIZE];
                        uint8_t hi = mem[(addr + 1) % MEM_SIZE];
                        cpu.BX = (hi << 8) | lo;
                        setZ(&cpu, cpu.BX);
                    } break;
                    default:
                        fprintf(stderr, "Invalid reg %u in LOAD\n", reg);
                        break;
                }

                if (trace) {
                    printf("LOAD reg=%u ← [%04X]\n", reg, addr);
                }
            } break;


            case 0x05: { // STORE [addr16], регистр
                uint8_t reg = mem[cpu.PC++ % MEM_SIZE];   // какой регистр сохраняем
                uint16_t addr = rd16(cpu.PC);
                cpu.PC += 2;

                switch (reg) {
                    case REG_AL:
                        mem[addr % MEM_SIZE] = cpu.AL;
                        break;
                    case REG_AH:
                        mem[addr % MEM_SIZE] = cpu.AH;
                        break;
                    case REG_BL:
                        mem[addr % MEM_SIZE] = cpu.BL;
                        break;
                    case REG_BH:
                        mem[addr % MEM_SIZE] = cpu.BH;
                        break;
                    case REG_AX:
                        mem[addr % MEM_SIZE] = cpu.AX & 0xFF;             // младший байт
                        mem[(addr + 1) % MEM_SIZE] = (cpu.AX >> 8) & 0xFF; // старший байт
                        break;
                    case REG_BX:
                        mem[addr % MEM_SIZE] = cpu.BX & 0xFF;
                        mem[(addr + 1) % MEM_SIZE] = (cpu.BX >> 8) & 0xFF;
                        break;
                    default:
                        fprintf(stderr, "Invalid reg %u in STORE\n", reg);
                        break;
                }

                if (trace) {
                    printf("STORE reg=%u → [%04X]\n", reg, addr);
                }
            } break;


            case 0x07:{ // SUB A,B
                uint8_t oper1 = mem[cpu.PC++ % MEM_SIZE];
                uint8_t oper2 = mem[cpu.PC++ % MEM_SIZE];
                uint8_t val = 0;
                short valid = 0;
                switch (oper2){
                    case REG_AL: val = cpu.AL; break;
                    case REG_AH: val = cpu.AH; break;
                    case REG_BL: val = cpu.BL; break;
                    case REG_BH: val = cpu.BH; break;
                    case REG_AX:
                    {
                        if (oper1 == REG_AL || oper1 == REG_BL || oper1 == REG_AH || oper1 == REG_BH){
                            printf("invalid operation:ADD 16 bit to 8 bit");
                        }
                        valid = 1;
                    }break;
                    case REG_BX:
                    {
                        if (oper1 == REG_AL || oper1 == REG_BL || oper1 == REG_AH || oper1 == REG_BH){
                            printf("invalid operation:ADD 16 bit to 8 bit");
                        }
                        valid = 1;
                    }break;
                default:
                    printf("invalid 8 bit reg");
                    break;
                }
                if (valid == 0){
                switch (oper1){
                    case REG_AL: cpu.AL -= val; break;
                    case REG_AH: cpu.AH -= val; break;
                    case REG_BL: cpu.BL -= val; break;
                    case REG_BH: cpu.BH -= val; break;     
                    case REG_AX: cpu.AX -= val; break;       
                    case REG_BX: cpu.BX -= val; break;    
                default:
                    printf("invalid reg");
                    break;
                    }
                }
            }break;

            case 0x08:{// ADD
                uint8_t oper1 = mem[cpu.PC++ % MEM_SIZE];
                uint8_t oper2 = mem[cpu.PC++ % MEM_SIZE];
                uint8_t val = 0;
                short valid = 0;
                switch (oper2){
                    case REG_AL: val = cpu.AL; break;
                    case REG_AH: val = cpu.AH; break;
                    case REG_BL: val = cpu.BL; break;
                    case REG_BH: val = cpu.BH; break;
                    case REG_AX:
                    {
                        if (oper1 == REG_AL || oper1 == REG_BL || oper1 == REG_AH || oper1 == REG_BH){
                            printf("invalid operation:ADD 16 bit to 8 bit");
                        }
                        valid = 1;
                    }break;
                    case REG_BX:
                    {
                        if (oper1 == REG_AL || oper1 == REG_BL || oper1 == REG_AH || oper1 == REG_BH){
                            printf("invalid operation:ADD 16 bit to 8 bit");
                        }
                        valid = 1;
                    }break;
                default:
                    printf("invalid 8 bit reg");
                    break;
                }
                if (valid == 0){
                switch (oper1){
                    case REG_AL: cpu.AL += val; break;
                    case REG_AH: cpu.AH += val; break;
                    case REG_BL: cpu.BL += val; break;
                    case REG_BH: cpu.BH += val; break;
                    case REG_AX: cpu.AX += val; break;       
                    case REG_BX: cpu.BX += val; break;    
                default:
                    printf("invalid reg");
                    break;
                    }
                }
            }break;

            case 0x09:{ // INC
            uint8_t reg = mem[cpu.PC++ % MEM_SIZE];
            switch(reg){
                case REG_AL: cpu.AL++; setZ(&cpu, cpu.AL); break;
                case REG_AH: cpu.AH++; setZ(&cpu, cpu.AH); break;
                case REG_BL: cpu.BL++; setZ(&cpu, cpu.BL); break;
                case REG_BH: cpu.BH++; setZ(&cpu, cpu.BH); break;
                case REG_AX: cpu.AX++; setZ(&cpu, (uint8_t)cpu.AX); break;
                case REG_BX: cpu.BX++; setZ(&cpu, (uint8_t)cpu.BX); break;
                default: fprintf(stderr, "Invalid reg %u in INC\n", reg); exit(1);
                }
            } break;

            case 0x0A:{ // DEC
            uint8_t reg = mem[cpu.PC++ % MEM_SIZE];
            switch(reg){
                case REG_AL: cpu.AL--; setZ(&cpu, cpu.AL); break;
                case REG_AH: cpu.AH--; setZ(&cpu, cpu.AH); break;
                case REG_BL: cpu.BL--; setZ(&cpu, cpu.BL); break;
                case REG_BH: cpu.BH--; setZ(&cpu, cpu.BH); break;
                case REG_AX: cpu.AX--; setZ(&cpu, (uint8_t)cpu.AX); break;
                case REG_BX: cpu.BX--; setZ(&cpu, (uint8_t)cpu.BX); break;
                default: fprintf(stderr, "Invalid reg %u in INC\n", reg); exit(1);
                }
            } break;

            case 0x0B: { // JMP addr16
                uint16_t addr = rd16(cpu.PC); cpu.PC = addr;
            } break;

            case 0x0C: { // JZ addr16
                uint16_t addr = rd16(cpu.PC); cpu.PC += 2;
                if (cpu.Z) cpu.PC = addr;
            } break;

            case 0x0D: { // JNZ addr16
                uint16_t addr = rd16(cpu.PC); cpu.PC += 2;
                if (!cpu.Z) cpu.PC = addr;
            } break;

            case 0x0E: // OUT
                printf("%c", cpu.AL);
            break;

            case 0xF9:{ //input
                cpu.AL = getch();
            } break;
            
            case 0xD0: { // CMP reg1, reg2
                uint8_t op1 = mem[cpu.PC++ % MEM_SIZE];
                uint8_t op2 = mem[cpu.PC++ % MEM_SIZE];
                uint16_t val1 = 0, val2 = 0;

                // читаем первое значение
                switch (op1) {
                    case REG_AL: val1 = cpu.AL; break;
                    case REG_AH: val1 = cpu.AH; break;
                    case REG_BL: val1 = cpu.BL; break;
                    case REG_BH: val1 = cpu.BH; break;
                    case REG_AX: val1 = cpu.AX; break;
                    case REG_BX: val1 = cpu.BX; break;
                    default:
                        val1 = op1; // если не регистр — считать числом
                        break;
                }

                // читаем второе значение
                switch (op2) {
                    case REG_AL: val2 = cpu.AL; break;
                    case REG_AH: val2 = cpu.AH; break;
                    case REG_BL: val2 = cpu.BL; break;
                    case REG_BH: val2 = cpu.BH; break;
                    case REG_AX: val2 = cpu.AX; break;
                    case REG_BX: val2 = cpu.BX; break;
                    default:
                        val2 = op2;
                        break;
                }

                cpu.Z = (val1 == val2);

                if (trace) {
                    printf("CMP: %u == %u -> Z=%u\n", val1, val2, cpu.Z);
                }
            } break;
   

            case 0xFF: // HALT
                if (trace) printf("HALT\n");
                return 0;
            
            default:
                fprintf(stderr, "Illegal opcode %02X at %04X\n", op, (cpu.PC-1)&0xFFFF);
                return 1;
        }
    }
}
