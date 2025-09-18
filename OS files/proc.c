#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MEM_SIZE 65535

#define NOP 0x00
#define LOAD_A(x) 0x01, (uint8_t)(x) //загрузить сразу в регистр A
#define LOAD_B(x) 0x02, (uint8_t)(x) //загрузить сращу в регистр В
#define LOADMEM_A(addr) 0x03, (uint16_t)((addr) & 0xFF), (uint16_t)(((addr) >> 8) & 0xFF) //загрузить из памяти 16 битной в регистр
#define LOADMEM_B(addr) 0x04, (uint16_t)((addr) & 0xFF), (uint16_t)(((addr) >> 8 )& 0xFF)
#define STA(addr) 0x05, (uint16_t)((addr) & 0xFF), (uint16_t)(((addr) >> 8) & 0xFF)
#define STB(addr) 0x06, (uint16_t)((addr) & 0xFF), (uint16_t)(((addr) >> 8) & 0xFF)
#define ADD 0x07 //складываем А и В
#define SUB 0x08 // вычесть A - B
#define INC 0x09 //inc A
#define DEC 0x0A // dec A
#define JMP(addr) 0x0B, (uint8_t)((addr) & 0xFF), (uint8_t)(((addr) >> 8) & 0xFF) //JMP 16 бит
#define JZ(addr) 0x0C, (uint8_t)((addr) & 0xFF),  (uint8_t)(((addr) >> 8) & 0xFF) //JZ 16 бит
#define JNZ(addr) 0x0D, (uint8_t)((addr) & 0xFF),  (uint8_t)(((addr) >> 8) & 0xFF) //JNZ 16 бит
#define PRINTAL 0x0E //выводим АL
#define PRINTA 0xAE //выводим АL
#define HLT 0xFF //халт
#define LDAL(x) 0xA1, (uint8_t)(x)
#define LDAH(x) 0xA2, (uint8_t)(x)
#define LDBL(x) 0xB1, (uint8_t)(x)
#define LDBH(x) 0xB2, (uint8_t)(x)

typedef struct 
{
    uint8_t AL, AH, BL, BH;
    uint8_t A, B;
    uint16_t AX, BX;
    uint16_t PC;
    uint8_t Z;
} CPU;

static uint8_t mem[MEM_SIZE];

static uint16_t rd16(uint16_t PC){ //читаем 16 битное число
    uint16_t lo = mem[PC];
    uint16_t hi = mem[(PC+1) & 0xFFFF];
    return (hi << 8) | lo;
}

static void setZ(CPU *cpu, uint8_t v) {cpu->Z = (v == 0); }

static void load_binary(const char *path){
    FILE *f = fopen(path, "rb");
    if (!f) {perror("fopen"); exit(1);}
    size_t n = fread(mem, 1, MEM_SIZE, f);  
    fclose(f);
    if (n == 0){fprintf(stderr, "empty\n"); exit(1);}
}

static void load_demo_program(void){
    uint8_t demo[] = {LOAD_A(0x42),PRINTA,LDAL(0x42),PRINTAL, 0xFF};
    memset(mem, 0, MEM_SIZE);
    memcpy(mem, demo, sizeof(demo));
}

int main(int argc, char **argv) {
    int trace = 0;
    const char *prog = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trace") == 0) trace = 1;
        else prog = argv[i];
    }

    if (prog) load_binary(prog); else load_demo_program();

    CPU cpu = { .A = 0, .B = 0, .PC = 0, .Z = 0, .AL = 0, .AH = 0, .BL =0, .BH = 0, .AX = 0, .BX = 0};

    for (;;) {
        uint8_t op = mem[cpu.PC++];
        if (trace) {
            printf("\n PC=%04u OP=%04X  A=%02u B=%02u Z=%02d AL=%02u AH=%02u BL=%02u BH=%02u AX=%04u BX=%04u\n",
                (cpu.PC-1) & 0xFFFF, op, cpu.A, cpu.B, cpu.Z, cpu.AL, cpu.AH, cpu.BL, cpu.BH, cpu.AX, cpu.BX);
        }
        switch (op) {
            case 0xA1:
                cpu.AL = mem[cpu.PC++];
                setZ(&cpu, cpu.AL);
                break;

            case 0xA2:
                cpu.AH = mem[cpu.PC++];
                setZ(&cpu, cpu.AH);
                break;
            case 0xA3:{
                uint16_t value = rd16(cpu.PC);
                cpu.PC += 2;
                cpu.AX = value;
                setZ(&cpu, cpu.AX);}
                break;
            case 0xB1: 
                cpu.BL = mem[cpu.PC++];
                setZ(&cpu, (uint8_t)cpu.BL);
                break; 
            case 0xB2: 
                cpu.BH = mem[cpu.PC++];
                setZ(&cpu, cpu.BH);
                break;  
            case 0xB3:{
                uint16_t value = rd16(cpu.PC);
                cpu.PC += 2;
                cpu.BX = value;
                setZ(&cpu, (uint8_t)cpu.BX);}
                break;
            case 0x00: /* NOP */ break;

            case 0x01: // LDA #imm
                cpu.A = mem[cpu.PC++];
                setZ(&cpu, cpu.A);
                break;

            case 0x02: // LDB #imm
                cpu.B = mem[cpu.PC++];
                setZ(&cpu, cpu.B);
                break;

            case 0x03: { // LDA [addr16]
                uint16_t addr = rd16(cpu.PC); cpu.PC += 2;
                cpu.A = mem[addr];
                setZ(&cpu, cpu.A);
            } break;

            case 0x04: { // LDB [addr16]
                uint16_t addr = rd16(cpu.PC); cpu.PC += 2;
                cpu.B = mem[addr];
                setZ(&cpu, cpu.B);
            } break;

            case 0x05: { // STA [addr16]
                uint16_t addr = rd16(cpu.PC); cpu.PC += 2;
                mem[addr] = cpu.A;
            } break;

            case 0x06: { // STB [addr16]
                uint16_t addr = rd16(cpu.PC); cpu.PC += 2;
                mem[addr] = cpu.B;
            } break;

            case 0x07: // ADD A,B
                cpu.A = (uint8_t)(cpu.A + cpu.B);
                setZ(&cpu, cpu.A);
                break;

            case 0x08: // SUB A,B
                cpu.A = (uint8_t)(cpu.A - cpu.B);
                setZ(&cpu, cpu.A);
                break;

            case 0x09: // INC A
                cpu.A = (uint8_t)(cpu.A + 1);
                setZ(&cpu, cpu.A);
                break;

            case 0x0A: // DEC A
                cpu.A = (uint8_t)(cpu.A - 1);
                setZ(&cpu, cpu.A);
                break;

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

            case 0x0E: // OUTAL
                if (cpu.AL >= 32){
                    printf("%c", (unsigned)cpu.AL);
                }
                else{
                    printf("%u", (unsigned)cpu.AL);
                }
            break;
            case 0xAE: //OUTA
                printf("%u", (unsigned)cpu.A);
                break;
            case 0xF9:{ //input
                cpu.AL = getch();
            } break;
            
            case 0xD0: // CMP A, B (сравнение A и B)
                if (cpu.A == cpu.B){
                    cpu.Z = 1;}
                else{
                    cpu.Z = 0;}
                break;    

            case 0xFF: // HALT
                if (trace) printf("HALT\n");
                return 0;
            
            default:
                fprintf(stderr, "Illegal opcode %02X at %04X\n", op, (cpu.PC-1)&0xFFFF);
                return 1;
        }
    }
}