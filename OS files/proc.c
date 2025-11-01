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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <conio.h>
#include "videocardemu.h"

#define MEM_SIZE 65536   // <- исправлено: 2^16

enum {
    REG_AL = 0,
    REG_AH = 1,
    REG_BL = 2,
    REG_BH = 3,
    REG_AX = 4,
    REG_BX = 5,
    REG_DS = 6,
    REG_CS = 7,
    REG_CX = 8,
    REG_PX = 9,
    REG_PY = 10
};

typedef struct 
{
    uint8_t AL, AH, BL, BH, CX, PX, PY;
    uint16_t AX, BX, DS, CS;
    uint16_t PC;
    uint8_t Z;
} CPU;

static inline void sync_ax_from_al_ah(CPU *cpu) {
    cpu->AX = ((uint16_t)cpu->AH << 8) | (uint16_t)cpu->AL;
}
static inline void sync_al_ah_from_ax(CPU *cpu) {
    cpu->AL = cpu->AX & 0xFF;
    cpu->AH = (cpu->AX >> 8) & 0xFF;
}
static inline void sync_bx_from_bl_bh(CPU *cpu) {
    cpu->BX = ((uint16_t)cpu->BH << 8) | (uint16_t)cpu->BL;
}
static inline void sync_bl_bh_from_bx(CPU *cpu) {
    cpu->BL = cpu->BX & 0xFF;
    cpu->BH = (cpu->BX >> 8) & 0xFF;
}

static uint8_t mem[MEM_SIZE];

static uint16_t rd16(CPU *cpu){ //читаем 16 битное число (безопасно по модулю MEM_SIZE)
    uint32_t addr = cpu->PC + cpu->CS;
    uint16_t lo = mem[addr % MEM_SIZE];
    uint16_t hi = mem[(addr + 1) % MEM_SIZE];
    return (hi << 8) | lo;
}

static void setZ8(CPU *cpu, uint8_t v) { cpu->Z = (v == 0); }
static void setZ16(CPU *cpu, uint16_t v) { cpu->Z = (v == 0); }


static void load_binary(const char *path){
    FILE *f = fopen(path, "rb");
    if (!f) {perror("fopen"); exit(1);}
    memset(mem, 0, MEM_SIZE);
    size_t n = fread(mem, 1, MEM_SIZE, f);  
    fclose(f);
    if (n == 0){fprintf(stderr, "empty\n"); exit(1);}
}

static void load_demo_program(void){
    uint8_t demo[] = {0x32,0x30,1,1,0x20,0x30,1,2,0x20,0x30,1,3,0x20,0x30,1,4,0x20,0x30,1,5,0x20,0x30,2,5,0x20,0x30,3,5,0x20,0x30,5,1,0x40,0x30,6,1,0x40,0x30,7,1,0x40,0x30,8,1,0x40,0x30,5,2,0x40,0x30,5,3,0x40,0x30,6,3,0x40,0x30,7,3,0x40,0x30,5,4,0x40,0x30,5,5,0x40,0x30,6,5,0x40,0x30,7,5,0x40,0x30,8,5,0x40,0x30,10,1,0x60,0x30,11,1,0x60,0x30,12,1,0x60,0x30,11,2,0x60,0x30,11,3,0x60,0x30,11,4,0x60,0x30,11,5,0x60,0x30,14,2,0x80,0x30,15,2,0x80,0x30,16,2,0x80,0x30,14,3,0x80,0x30,16,3,0x80,0x30,14,4,0x80,0x30,15,4,0x80,0x30,16,4,0x80,0x30,18,2,0xA0,0x30,19,2,0xA0,0x30,20,2,0xA0,0x30,18,3,0xA0,0x30,18,4,0xA0,0x30,19,4,0xA0,0x30,20,4,0xA0,0x30,20,5,0xA0,0x31,0xF9,0x32,0x31,0xF9,0x0B,0x00,0x00,0xFF};
    memset(mem, 0, MEM_SIZE);
    memcpy(mem, demo, sizeof(demo));
}

int main(int argc, char **argv) {
    printf("Welcome to Letos 0.0.3!\n");
    int trace = 0;
    const char *prog = NULL;
    vga_init();
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trace") == 0) trace = 1;
        else prog = argv[i];
    }

    if (prog) load_binary(prog); else load_demo_program();

    CPU cpu = {.PC = 0, .Z = 0, .AL = 0, .AH = 0, .BL =0, .BH = 0, .AX = 0, .BX = 0, .DS = 0x1000, .CS = 0, .CX = 0, .PX = 0, .PY = 0};

    for (;;) {
        uint8_t op = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
        if (trace) {
            printf("\n PC=%04u OP=%02X Z=%02d AL=%02u AH=%02u BL=%02u BH=%02u AX=%04u BX=%04u DS=%04u CS=%04u CX=%02u PX=%02u PY=%02u\n",
                (cpu.PC-1 + cpu.CS) & 0xFFFF, op, cpu.Z, cpu.AL, cpu.AH, cpu.BL, cpu.BH, cpu.AX, cpu.BX, cpu.DS, cpu.CS, cpu.CX, cpu.PX, cpu.PY);
        }
        
        switch (op) {
            case 0x30: { //установка пикселя на дисплее VGA
                uint8_t x = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t y = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t reg = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                switch(x){
                    case REG_PX: x = cpu.PX; break;
                };
                switch(y){
                    case REG_PY: y = cpu.PY; break;
                };
                switch(reg){
                    case REG_AL: vga_set_pixel(x, y, cpu.AL); break;
                    case REG_AH: vga_set_pixel(x, y, cpu.AH); break;
                    case REG_BL: vga_set_pixel(x, y, cpu.BL); break;
                    case REG_BH: vga_set_pixel(x, y, cpu.BH); break;
                    case REG_AX: vga_set_pixel(x, y, cpu.AX); break;
                    case REG_BX: vga_set_pixel(x, y, cpu.BX); break;
                    case REG_CX: vga_set_pixel(x, y, cpu.CX); break;
                    default: vga_set_pixel(x, y, reg);
                };
                if (trace) printf("set pixel in %2X, %2X = %2X", x, y, reg);
            }break;
            case 0x31: { //рендер дисплея (вывод пикселей на экран)
                vga_render();
            }break;
            case 0x32:{ //инициализация(можно использовать как сброс всех пикселей на черный)
                vga_init();
            }break;
            case 0x20: {
                uint8_t flags = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t dst = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t src = mem[cpu.PC++ % MEM_SIZE + cpu.CS];


                if (flags == 0) { // 8-bit
                    uint8_t val = 0;
                    switch (src) {
                        case REG_AL: val = cpu.AL; break;
                        case REG_AH: val = cpu.AH; break;
                        case REG_BL: val = cpu.BL; break;
                        case REG_BH: val = cpu.BH; break;
                        case REG_CX: val = cpu.CX; break;
                        case REG_PX: val = cpu.PX; break;
                        case REG_PY: val = cpu.PY; break;
                        default:if (src <= REG_PY) {
                                // это регистр
                            } else {
                                val = src; // это значение
                            }
                        break;
                    }
                    switch (dst) {
                        case REG_AL: cpu.AL = val; sync_ax_from_al_ah(&cpu); break;
                        case REG_AH: cpu.AH = val; sync_ax_from_al_ah(&cpu); break;
                        case REG_BL: cpu.BL = val; sync_bx_from_bl_bh(&cpu); break;
                        case REG_BH: cpu.BH = val; sync_bx_from_bl_bh(&cpu); break;
                        case REG_CX: cpu.CX = src; break;
                        case REG_PX: cpu.PX = val; break;
                        case REG_PY: cpu.PY = val; break;
                        default: fprintf(stderr, "Invalid 8-bit dst reg %u\n", dst); break;
                    }
                    if (trace) printf("MOV flags=%u dst=%u src=%u\n", flags, dst, src);
                } else if (flags == 1) { // 16-bit
                    uint16_t val = 0;
                    uint8_t src2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                    switch (src) {
                        case REG_AX: val = cpu.AX; break;
                        case REG_BX: val = cpu.BX; break;
                        case REG_DS: val = cpu.DS; break;
                        case REG_CS: val = cpu.CS; break;
                        default: if (src <= REG_CS) {
                                // это регистр
                            } else {
                                val = (src << 8) | src2; // это значение
                            }
                        break;
                    }
                    switch (dst) {
                        case REG_AX: cpu.AX = val; sync_al_ah_from_ax(&cpu); break;
                        case REG_BX: cpu.BX = val; sync_bl_bh_from_bx(&cpu); break;
                        case REG_DS: cpu.DS = val; break;
                        case REG_CS: cpu.CS = val; break;
                        default: fprintf(stderr, "Invalid 16-bit dst reg %u\n", dst); break;
                    }
                    if (trace) printf("MOV flags=%u dst=%u src=%04u\n", flags, dst, (src << 8) | src2);
                }
            } break;


            case 0x00: /* NOP */ break;

            case 0x04: { // LOAD регистр, [addr16]
                uint8_t reg = mem[cpu.PC++ % MEM_SIZE + cpu.CS];   // какой регистр загружаем
                uint16_t addr = rd16(&cpu); 
                cpu.PC += 2;

                switch (reg) {
                    case REG_AL:
                        cpu.AL = mem[addr % MEM_SIZE + cpu.DS];
                        setZ8(&cpu, cpu.AL);
                        sync_ax_from_al_ah(&cpu);
                        break;
                    case REG_AH:
                        cpu.AH = mem[addr % MEM_SIZE + cpu.DS];
                        setZ8(&cpu, cpu.AH);
                        sync_ax_from_al_ah(&cpu);
                        break;
                    case REG_BL:
                        cpu.BL = mem[addr % MEM_SIZE + cpu.DS];
                        setZ8(&cpu, cpu.BL);
                        sync_bx_from_bl_bh(&cpu);
                        break;
                    case REG_BH:
                        cpu.BH = mem[addr % MEM_SIZE + cpu.DS];
                        setZ8(&cpu, cpu.BH);
                        sync_bx_from_bl_bh(&cpu);
                        break;
                    case REG_AX: {
                        uint8_t lo = mem[addr % MEM_SIZE + cpu.DS];
                        uint8_t hi = mem[(addr + 1) % MEM_SIZE + cpu.DS];
                        cpu.AX = (hi << 8) | lo;
                        setZ16(&cpu, cpu.AX);
                        sync_al_ah_from_ax(&cpu);
                    } break;
                    case REG_BX: {
                        uint8_t lo = mem[addr % MEM_SIZE + cpu.DS];
                        uint8_t hi = mem[(addr + 1) % MEM_SIZE + cpu.DS];
                        cpu.BX = (hi << 8) | lo;
                        setZ16(&cpu, cpu.BX);
                        sync_bl_bh_from_bx(&cpu);
                    } break;
                    case REG_DS: {
                        uint8_t lo = mem[addr % MEM_SIZE + cpu.DS];
                        uint8_t hi = mem[(addr + 1) % MEM_SIZE + cpu.DS];
                        cpu.DS = (hi << 8) | lo;
                        setZ16(&cpu, cpu.DS);
                    } break;
                    case REG_CS: {
                        uint8_t lo = mem[addr % MEM_SIZE + cpu.DS];
                        uint8_t hi = mem[(addr + 1) % MEM_SIZE + cpu.DS];
                        cpu.CS = (hi << 8) | lo;
                        setZ16(&cpu, cpu.CS);
                    } break;
                    case REG_CX:{
                        cpu.CX = mem[addr % MEM_SIZE + cpu.DS];
                        setZ8(&cpu, cpu.CX);
                    }break;
                    case REG_PX:{
                        cpu.PX = mem[addr % MEM_SIZE + cpu.DS];
                        setZ8(&cpu, cpu.PX);
                    }break;
                    case REG_PY:{
                        cpu.PY = mem[addr % MEM_SIZE + cpu.DS];
                        setZ8(&cpu, cpu.PY);
                    }break;
                    default:
                        fprintf(stderr, "Invalid reg %u in LOAD\n", reg);
                        break;
                }
                if (trace) {
                    uint16_t pc_now = (cpu.PC-1 + cpu.CS) & 0xFFFF;
                    printf(">> ENTER LOAD at PC=%04X raw next bytes: %02X %02X %02X\n",
                        pc_now,
                        mem[cpu.PC % MEM_SIZE],
                        mem[(cpu.PC+1) % MEM_SIZE],
                        mem[(cpu.PC+2) % MEM_SIZE]);
                    fflush(stdout);
                }

            } break;


            case 0x05: { // STORE [addr16], регистр
                uint8_t reg = mem[cpu.PC++ % MEM_SIZE + cpu.CS];   // 1 байт — какой регистр сохраняем
                uint16_t addr = rd16(&cpu);             // читаем 2 байта адреса
                cpu.PC += 2;

                switch (reg) {
                    case REG_AL:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.AL;
                        break;
                    case REG_AH:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.AH;
                        break;
                    case REG_BL:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.BL;
                        break;
                    case REG_BH:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.BH;
                        break;
                    case REG_AX:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.AX & 0xFF;             // младший байт
                        mem[(addr + 1) % MEM_SIZE + cpu.DS] = (cpu.AX >> 8) & 0xFF; // старший байт
                        break;
                    case REG_BX:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.BX & 0xFF;
                        mem[(addr + 1) % MEM_SIZE + cpu.DS] = (cpu.BX >> 8) & 0xFF;
                        break;
                    case REG_DS:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.DS & 0xFF;
                        mem[(addr + 1) % MEM_SIZE + cpu.DS] = (cpu.DS >> 8) & 0xFF;
                        break;
                    case REG_CS:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.CS & 0xFF;
                        mem[(addr + 1) % MEM_SIZE + cpu.DS] = (cpu.CS >> 8) & 0xFF;
                        break;    
                    case REG_CX:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.CX;
                        break;    
                    case REG_PX:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.PX;
                        break;      
                    case REG_PY:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.PY;
                        break;      
                    default:
                        mem[addr % MEM_SIZE + cpu.DS] = reg;
                        break;
                }
                if (trace) {
                    uint16_t pc_now = (cpu.PC-1+cpu.CS) & 0xFFFF;
                    printf(">> ENTER STORE at PC=%04X raw next bytes: %02X %02X %02X\n",
                        pc_now,
                        mem[cpu.PC % MEM_SIZE + cpu.CS],
                        mem[(cpu.PC+1) % MEM_SIZE + cpu.CS],
                        mem[(cpu.PC+2) % MEM_SIZE + cpu.CS]);
                    fflush(stdout);
                }
            } break;



            case 0x07:{ // SUB A,B
                uint8_t oper1 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t oper2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t val = 0;
                short valid = 0;
                switch (oper2){
                    case REG_AL: val = cpu.AL; break;
                    case REG_AH: val = cpu.AH; break;
                    case REG_BL: val = cpu.BL; break;
                    case REG_BH: val = cpu.BH; break;
                    case REG_CX: val = cpu.CX; break;
                    case REG_PX: val = cpu.PX; break;
                    case REG_PY: val = cpu.PY; break;
                    case REG_AX:
                    {
                        if (oper1 == REG_AL || oper1 == REG_BL || oper1 == REG_AH || oper1 == REG_BH || oper1 == REG_CX || oper1 == REG_PX || oper1 == REG_PY){
                            printf("invalid operation:SUB 16 bit from 8 bit");
                            valid = 1;
                        }
                        else{
                            val = cpu.AX;
                        }
                    }break;
                    case REG_BX:
                    {
                        if (oper1 == REG_AL || oper1 == REG_BL || oper1 == REG_AH || oper1 == REG_BH || oper1 == REG_CX || oper1 == REG_PX || oper1 == REG_PY){
                            printf("invalid operation:SUB 16 bit from 8 bit");
                            valid = 1;
                        }
                        else{
                            val = cpu.BX;
                        }
                    }break;
                default:
                    val = oper2;
                    break;
                }
                if (valid == 0){
                switch (oper1){
                    case REG_AL: cpu.AL -= val; setZ8(&cpu, cpu.AL); sync_ax_from_al_ah(&cpu); break;
                    case REG_AH: cpu.AH -= val; setZ8(&cpu, cpu.AH); sync_ax_from_al_ah(&cpu); break;
                    case REG_BL: cpu.BL -= val; setZ8(&cpu, cpu.BL); sync_bx_from_bl_bh(&cpu); break;
                    case REG_BH: cpu.BH -= val; setZ8(&cpu, cpu.BH); sync_bx_from_bl_bh(&cpu); break;     
                    case REG_CX: cpu.CX -= val; setZ8(&cpu, cpu.CX); break;  
                    case REG_PX: cpu.PX -= val; setZ8(&cpu, cpu.PX); break;  
                    case REG_PY: cpu.PY -= val; setZ8(&cpu, cpu.PY); break;  
                    case REG_AX: cpu.AX -= val; setZ16(&cpu, cpu.AX); sync_al_ah_from_ax(&cpu); break;       
                    case REG_BX: cpu.BX -= val; setZ16(&cpu, cpu.BX); sync_bl_bh_from_bx(&cpu); break;    
                default:
                    printf("invalid reg");
                    break;
                    }
                }
            }break;

            case 0x08:{// ADD
                uint8_t oper1 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t oper2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t val = 0;
                short valid = 0;
                switch (oper2){
                    case REG_AL: val = cpu.AL; break;
                    case REG_AH: val = cpu.AH; break;
                    case REG_BL: val = cpu.BL; break;
                    case REG_BH: val = cpu.BH; break;
                    case REG_CX: val = cpu.CX; break;
                    case REG_PX: val = cpu.PX; break;
                    case REG_PY: val = cpu.PY; break;
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
                    val = oper2;
                    break;
                }
                if (valid == 0){
                switch (oper1){
                    case REG_AL: cpu.AL += val; setZ8(&cpu, cpu.AL); sync_ax_from_al_ah(&cpu); break;
                    case REG_AH: cpu.AH += val; setZ8(&cpu, cpu.AH); sync_ax_from_al_ah(&cpu); break;
                    case REG_BL: cpu.BL += val; setZ8(&cpu, cpu.BL); sync_bx_from_bl_bh(&cpu); break;
                    case REG_BH: cpu.BH += val; setZ8(&cpu, cpu.BH); sync_bx_from_bl_bh(&cpu); break;    
                    case REG_CX: cpu.CX += val; setZ8(&cpu, cpu.CX); break;  
                    case REG_PX: cpu.PX += val; setZ8(&cpu, cpu.PX); break;  
                    case REG_PY: cpu.PY += val; setZ8(&cpu, cpu.PY); break;   
                    case REG_AX: cpu.AX += val; setZ16(&cpu, cpu.AX); sync_al_ah_from_ax(&cpu); break;       
                    case REG_BX: cpu.BX += val; setZ16(&cpu, cpu.BX); sync_bl_bh_from_bx(&cpu); break;       
                default:
                    printf("invalid reg");
                    break;
                    }
                }
            }break;

            case 0x09:{ // INC
            uint8_t reg = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
            switch(reg){
                case REG_AL: cpu.AL++; setZ8(&cpu, cpu.AL); sync_ax_from_al_ah(&cpu); break;
                case REG_AH: cpu.AH++; setZ8(&cpu, cpu.AH); sync_ax_from_al_ah(&cpu); break;
                case REG_BL: cpu.BL++; setZ8(&cpu, cpu.BL); sync_bx_from_bl_bh(&cpu); break;
                case REG_BH: cpu.BH++; setZ8(&cpu, cpu.BH); sync_bx_from_bl_bh(&cpu); break;     
                case REG_CX: cpu.CX++; setZ8(&cpu, cpu.CX); break;     
                case REG_PX: cpu.PX++; setZ8(&cpu, cpu.PX); break;   
                case REG_PY: cpu.PY++; setZ8(&cpu, cpu.PY); break;   
                case REG_AX: cpu.AX++; setZ16(&cpu, cpu.AX); sync_al_ah_from_ax(&cpu); break;       
                case REG_BX: cpu.BX++; setZ16(&cpu, cpu.BX); sync_bl_bh_from_bx(&cpu); break;    
                default: fprintf(stderr, "Invalid reg %u in INC\n", reg); exit(1);
                }
            } break;

            case 0x0A:{ // DEC
            uint8_t reg = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
            switch(reg){
                case REG_AL: cpu.AL--; setZ8(&cpu, cpu.AL); sync_ax_from_al_ah(&cpu); break;
                case REG_AH: cpu.AH--; setZ8(&cpu, cpu.AH); sync_ax_from_al_ah(&cpu); break;
                case REG_BL: cpu.BL--; setZ8(&cpu, cpu.BL); sync_bx_from_bl_bh(&cpu); break;
                case REG_BH: cpu.BH--; setZ8(&cpu, cpu.BH); sync_bx_from_bl_bh(&cpu); break;     
                case REG_CX: cpu.CX--; setZ8(&cpu, cpu.CX); break;     
                case REG_PX: cpu.PX--; setZ8(&cpu, cpu.PX); break;   
                case REG_PY: cpu.PY--; setZ8(&cpu, cpu.PY); break;   
                case REG_AX: cpu.AX--; setZ16(&cpu, cpu.AX); sync_al_ah_from_ax(&cpu); break;       
                case REG_BX: cpu.BX--; setZ16(&cpu, cpu.BX); sync_bl_bh_from_bx(&cpu); break;    
                default: fprintf(stderr, "Invalid reg %u in INC\n", reg); exit(1);
                }
            } break;

            case 0x0B: { // JMP addr16
                uint16_t addr = rd16(&cpu); cpu.PC = addr;
            } break;

            case 0x0C: { // JZ addr16
                uint16_t addr = rd16(&cpu); cpu.PC += 2;
                if (cpu.Z) cpu.PC = addr;
            } break;

            case 0x0D: { // JNZ addr16
                uint16_t addr = rd16(&cpu); cpu.PC += 2;
                if (!cpu.Z) cpu.PC = addr;
            } break;

            case 0x0E: // OUT
                printf("%c", cpu.AL);
            break;
            case 0x10:{ // PRINT_STR - вывод строки из памяти
                uint16_t stroke = cpu.BH + cpu.DS;
                if (trace){printf("=== DEBUG PRINT_STR ===\n");
                printf("BH=%02X, DS=%04X, calculated address=%04X\n", cpu.BH, cpu.DS, stroke);
                printf("Memory dump at %04X: ", stroke);
                for (int i = 0; i < 6; i++) {
                    uint32_t addr = (stroke + i) % MEM_SIZE;
                    uint8_t ch = mem[addr];
                    printf("[%04X]=%02X(%c) ", addr, ch, (ch >= 32 && ch < 127) ? ch : '.');
                }
                printf("\n");}
                
                int count = 0;
                while (count < 100) {
                    uint32_t addr = stroke % MEM_SIZE;
                    uint8_t ch = mem[addr];
                    if (trace){printf("Reading [%04X] = %02X -> ", addr, ch);}
                    
                    
                    if (ch == 0) {
                        if (trace){printf("END (zero terminator)\n");}
                        
                        break;
                    }
                    if (trace){printf("output '%c'\n", ch);}
                    
                    printf("%c", ch);
                    stroke++;
                    count++;
                    
                    if (stroke >= MEM_SIZE) break;
                }
                if (trace){printf("=== END PRINT_STR ===\n");}
                
            } break;
            case 0xD0: { // CMP reg1, reg2
                uint8_t op1 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t op2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint16_t val1 = 0, val2 = 0;

                // читаем первое значение
                switch (op1) {
                    case REG_AL: val1 = cpu.AL; break;
                    case REG_AH: val1 = cpu.AH; break;
                    case REG_BL: val1 = cpu.BL; break;
                    case REG_BH: val1 = cpu.BH; break;
                    case REG_CX: val1 = cpu.CX; break;
                    case REG_PX: val1 = cpu.PX; break;
                    case REG_PY: val1 = cpu.PY; break;
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
                    case REG_CX: val2 = cpu.CX; break;
                    case REG_PX: val2 = cpu.PX; break;
                    case REG_PY: val2 = cpu.PY; break;
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
   
            case 0xD1: { // < reg1, reg2
                uint8_t op1 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t op2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint16_t val1 = 0, val2 = 0;

                // читаем первое значение
                switch (op1) {
                    case REG_AL: val1 = cpu.AL; break;
                    case REG_AH: val1 = cpu.AH; break;
                    case REG_BL: val1 = cpu.BL; break;
                    case REG_BH: val1 = cpu.BH; break;
                    case REG_CX: val1 = cpu.CX; break;
                    case REG_PX: val1 = cpu.PX; break;
                    case REG_PY: val1 = cpu.PY; break;
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
                    case REG_CX: val2 = cpu.CX; break;
                    case REG_PX: val2 = cpu.PX; break;
                    case REG_PY: val2 = cpu.PY; break;
                    case REG_AX: val2 = cpu.AX; break;
                    case REG_BX: val2 = cpu.BX; break;
                    default:
                        val2 = op2;
                        break;
                }

                cpu.Z = (val1 < val2);

                if (trace) {
                    printf("%u < %u -> Z=%u\n", val1, val2, cpu.Z);
                }
            } break;

            case 0xD2: { // > reg1, reg2
                uint8_t op1 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t op2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint16_t val1 = 0, val2 = 0;

                // читаем первое значение
                switch (op1) {
                    case REG_AL: val1 = cpu.AL; break;
                    case REG_AH: val1 = cpu.AH; break;
                    case REG_BL: val1 = cpu.BL; break;
                    case REG_BH: val1 = cpu.BH; break;
                    case REG_CX: val1 = cpu.CX; break;
                    case REG_PX: val1 = cpu.PX; break;
                    case REG_PY: val1 = cpu.PY; break;
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
                    case REG_CX: val2 = cpu.CX; break;
                    case REG_PX: val2 = cpu.PX; break;
                    case REG_PY: val2 = cpu.PY; break;
                    case REG_AX: val2 = cpu.AX; break;
                    case REG_BX: val2 = cpu.BX; break;
                    default:
                        val2 = op2;
                        break;
                }

                cpu.Z = (val1 > val2);

                if (trace) {
                    printf("%u > %u -> Z=%u\n", val1, val2, cpu.Z);
                }
            } break;
            
            case 0xF9:{ //input
                int ch = getch();
                cpu.AL = ch;
                sync_ax_from_al_ah(&cpu);
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