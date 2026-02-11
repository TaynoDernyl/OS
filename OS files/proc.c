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

#define MEM_SIZE 65536
#define version "0.0.7 NEW ERA"

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
    REG_PY = 10,
    REG_SP = 11,
    REG_PC = 255
};

typedef struct 
{
    uint8_t AL, AH, BL, BH, PX, PY;
    uint16_t AX, BX, DS, CS, CX;
    uint16_t PC, SP;
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

static uint16_t rd16(CPU *cpu){
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
    uint8_t demo[] = {0x20,0x01,0x00,0x04,0x00,0x10,0xC0,0x01,0x01,0x04,0xFF};
    memset(mem, 0, MEM_SIZE);
    memcpy(mem, demo, sizeof(demo));
}

int main(int argc, char **argv) {
    printf("Welcome to Letos %s!\n", version);
    int trace = 0;
    const char *prog = NULL;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trace") == 0) trace = 1;
        else prog = argv[i];
    }

    if (prog) load_binary(prog); else load_demo_program();

    CPU cpu = {.PC = 0, .Z = 0, .AL = 0, .AH = 0, .BL =0, .BH = 0, .AX = 0, .BX = 0, .DS = 0x1000, .CS = 0, .CX = 0, .PX = 0, .PY = 0, .SP = 0x2000};

    for (;;) {
        uint8_t op = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
        if (trace) {
            printf("\n PC=%04u OP=%02X Z=%02d AL=%02u AH=%02u BL=%02u BH=%02u AX=%04u BX=%04u DS=%04u CS=%04u CX=%04u PX=%02u PY=%02u, Stack=%04u\n",
                (cpu.PC-1 + cpu.CS) & 0xFFFF, op, cpu.Z, cpu.AL, cpu.AH, cpu.BL, cpu.BH, cpu.AX, cpu.BX, cpu.DS, cpu.CS, cpu.CX, cpu.PX, cpu.PY, cpu.SP);
        }
        
        switch (op) {
            case 0x30: {
                uint8_t x = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t y = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t reg = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                if (x == REG_PX) x = cpu.PX;
                if (y == REG_PY) y = cpu.PY;
                
                uint8_t value;
                switch(reg){
                    case REG_AL: value = cpu.AL; break;
                    case REG_AH: value = cpu.AH; break;
                    case REG_BL: value = cpu.BL; break;
                    case REG_BH: value = cpu.BH; break;
                    case REG_AX: value = cpu.AX; break;
                    case REG_BX: value = cpu.BX; break;
                    case REG_CX: value = cpu.CX; break;
                    default: value = reg; break;
                };
                vga_set_pixel(x, y, value);
                if (trace) printf("set pixel in %2X, %2X = %2X", x, y, value);
            } break;
            
            case 0x31: {
                vga_render();
            } break;
            
            case 0x32: {
                vga_init();
            } break;
            
            case 0x20: {
                uint8_t flags = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t IR = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t dst = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t src = mem[cpu.PC++ % MEM_SIZE + cpu.CS];

                if (flags == 0) {
                    uint8_t val = 0;
                    if (IR == 1){
                        switch (src) {
                            case REG_AL: val = cpu.AL; break;
                            case REG_AH: val = cpu.AH; break;
                            case REG_BL: val = cpu.BL; break;
                            case REG_BH: val = cpu.BH; break;
                            case REG_PX: val = cpu.PX; break;
                            case REG_PY: val = cpu.PY; break;
                        }
                    } else if(IR == 0){
                        val = src;
                    }    
                    switch (dst) {
                        case REG_AL: cpu.AL = val; sync_ax_from_al_ah(&cpu); break;
                        case REG_AH: cpu.AH = val; sync_ax_from_al_ah(&cpu); break;
                        case REG_BL: cpu.BL = val; sync_bx_from_bl_bh(&cpu); break;
                        case REG_BH: cpu.BH = val; sync_bx_from_bl_bh(&cpu); break;
                        case REG_PX: cpu.PX = val; break;
                        case REG_PY: cpu.PY = val; break;
                        default: fprintf(stderr, "Invalid 8-bit dst reg %u\n", dst); break;
                    }
                    if (trace) printf("MOV flags=%u IR=%d dst=%u src=%u\n", flags, IR, dst, src);
                } else if (flags == 1) {
                    uint16_t val = 0;
                    if(IR == 1){
                        switch (src) {
                            case REG_AX: val = cpu.AX; break;
                            case REG_BX: val = cpu.BX; break;
                            case REG_CX: val = cpu.CX; break;
                            case REG_DS: val = cpu.DS; break;
                            case REG_CS: val = cpu.CS; break;
                        }
                    }
                    else if(IR == 0){
                        uint8_t src2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                        val = (val & 0xFF00) | src;
                        val = (val & 0x00FF) | ((uint16_t)src2 << 8);
                    }   
                    switch (dst) {
                        case REG_AX: cpu.AX = val; sync_al_ah_from_ax(&cpu); break;
                        case REG_BX: cpu.BX = val; sync_bl_bh_from_bx(&cpu); break;
                        case REG_CX: cpu.CX = val; break;
                        case REG_DS: cpu.DS = val; break;
                        case REG_CS: cpu.CS = val; break;
                        default: fprintf(stderr, "Invalid 16-bit dst reg %u\n", dst); break;
                    }
                    if (trace) printf("MOV flags=%u IR=%d dst=%u src=%04u\n", flags, IR, dst, val);
                }
            } break;

            case 0x00: break;

            case 0x04: {
                uint8_t reg = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
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
                    case REG_CX: {
                        uint8_t lo = mem[addr % MEM_SIZE + cpu.DS];
                        uint8_t hi = mem[(addr + 1) % MEM_SIZE + cpu.DS];
                        cpu.CX = (hi << 8) | lo;
                        setZ16(&cpu, cpu.CX);
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
                    case REG_PX:
                        cpu.PX = mem[addr % MEM_SIZE + cpu.DS];
                        setZ8(&cpu, cpu.PX);
                        break;
                    case REG_PY:
                        cpu.PY = mem[addr % MEM_SIZE + cpu.DS];
                        setZ8(&cpu, cpu.PY);
                        break;
                    default:
                        fprintf(stderr, "Invalid reg %u in LOAD\n", reg);
                        break;
                }
            } break;

            case 0x05: {
                uint8_t reg = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint16_t addr = rd16(&cpu);
                cpu.PC += 2;
                if(trace){printf("STORE in %d", addr);}

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
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.AX & 0xFF;
                        mem[(addr + 1) % MEM_SIZE + cpu.DS] = (cpu.AX >> 8) & 0xFF;
                        break;
                    case REG_BX:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.BX & 0xFF;
                        mem[(addr + 1) % MEM_SIZE + cpu.DS] = (cpu.BX >> 8) & 0xFF;
                        break;
                    case REG_CX:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.CX & 0xFF;
                        mem[(addr + 1) % MEM_SIZE + cpu.DS] = (cpu.CX >> 8) & 0xFF;
                        break;
                    case REG_DS:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.DS & 0xFF;
                        mem[(addr + 1) % MEM_SIZE + cpu.DS] = (cpu.DS >> 8) & 0xFF;
                        break;
                    case REG_CS:
                        mem[addr % MEM_SIZE + cpu.DS] = cpu.CS & 0xFF;
                        mem[(addr + 1) % MEM_SIZE + cpu.DS] = (cpu.CS >> 8) & 0xFF;
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
            } break;

            case 0x07: {
                uint8_t IR = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t oper1 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t oper2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint16_t val = 0;
                int valid = 1;

                if (IR == 1){
                    switch (oper2){
                        case REG_AL: val = cpu.AL; break;
                        case REG_AH: val = cpu.AH; break;
                        case REG_BL: val = cpu.BL; break;
                        case REG_BH: val = cpu.BH; break;
                        case REG_PX: val = cpu.PX; break;
                        case REG_PY: val = cpu.PY; break;
                        case REG_AX: val = cpu.AX; break;
                        case REG_BX: val = cpu.BX; break;
                        case REG_CX: val = cpu.CX; break;
                        default: val = oper2; break;
                    }
                }
                else if(IR == 0){
                    val = oper2;
                }

                if ((oper1 <= REG_PY) && (val > 255)) {
                    printf("invalid operation:SUB 16 bit from 8 bit");
                    valid = 0;
                }

                if (valid) {
                    switch (oper1){
                        case REG_AL: cpu.AL -= val; setZ8(&cpu, cpu.AL); sync_ax_from_al_ah(&cpu); break;
                        case REG_AH: cpu.AH -= val; setZ8(&cpu, cpu.AH); sync_ax_from_al_ah(&cpu); break;
                        case REG_BL: cpu.BL -= val; setZ8(&cpu, cpu.BL); sync_bx_from_bl_bh(&cpu); break;
                        case REG_BH: cpu.BH -= val; setZ8(&cpu, cpu.BH); sync_bx_from_bl_bh(&cpu); break;     
                        case REG_PX: cpu.PX -= val; setZ8(&cpu, cpu.PX); break;  
                        case REG_PY: cpu.PY -= val; setZ8(&cpu, cpu.PY); break;  
                        case REG_AX: cpu.AX -= val; setZ16(&cpu, cpu.AX); sync_al_ah_from_ax(&cpu); break;       
                        case REG_BX: cpu.BX -= val; setZ16(&cpu, cpu.BX); sync_bl_bh_from_bx(&cpu); break;    
                        case REG_CX: cpu.CX -= val; setZ16(&cpu, cpu.CX); break;
                        default: printf("invalid reg"); break;
                    }
                }
            } break;

            case 0x08: {
                uint8_t IR = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t oper1 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t oper2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint16_t val = 0;
                int valid = 1;
                if (IR == 1){
                    switch (oper2){
                        case REG_AL: val = cpu.AL; break;
                        case REG_AH: val = cpu.AH; break;
                        case REG_BL: val = cpu.BL; break;
                        case REG_BH: val = cpu.BH; break;
                        case REG_PX: val = cpu.PX; break;
                        case REG_PY: val = cpu.PY; break;
                        case REG_AX: val = cpu.AX; break;
                        case REG_BX: val = cpu.BX; break;
                        case REG_CX: val = cpu.CX; break;
                        default: val = oper2; break;
                    }
                }
                else if(IR == 0){
                    val = oper2;
                }

                if ((oper1 <= REG_PY) && (val > 255)) {
                    printf("invalid operation:ADD 16 bit to 8 bit");
                    valid = 0;
                }

                if (valid) {
                    switch (oper1){
                        case REG_AL: cpu.AL += val; setZ8(&cpu, cpu.AL); sync_ax_from_al_ah(&cpu); break;
                        case REG_AH: cpu.AH += val; setZ8(&cpu, cpu.AH); sync_ax_from_al_ah(&cpu); break;
                        case REG_BL: cpu.BL += val; setZ8(&cpu, cpu.BL); sync_bx_from_bl_bh(&cpu); break;
                        case REG_BH: cpu.BH += val; setZ8(&cpu, cpu.BH); sync_bx_from_bl_bh(&cpu); break;    
                        case REG_PX: cpu.PX += val; setZ8(&cpu, cpu.PX); break;  
                        case REG_PY: cpu.PY += val; setZ8(&cpu, cpu.PY); break;   
                        case REG_AX: cpu.AX += val; setZ16(&cpu, cpu.AX); sync_al_ah_from_ax(&cpu); break;       
                        case REG_BX: cpu.BX += val; setZ16(&cpu, cpu.BX); sync_bl_bh_from_bx(&cpu); break;       
                        case REG_CX: cpu.CX += val; setZ16(&cpu, cpu.CX); break;
                        default: printf("invalid reg"); break;
                    }
                }
            } break;

            case 0x09: {
                uint8_t reg = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                switch(reg){
                    case REG_AL: cpu.AL++; setZ8(&cpu, cpu.AL); sync_ax_from_al_ah(&cpu); break;
                    case REG_AH: cpu.AH++; setZ8(&cpu, cpu.AH); sync_ax_from_al_ah(&cpu); break;
                    case REG_BL: cpu.BL++; setZ8(&cpu, cpu.BL); sync_bx_from_bl_bh(&cpu); break;
                    case REG_BH: cpu.BH++; setZ8(&cpu, cpu.BH); sync_bx_from_bl_bh(&cpu); break;     
                    case REG_PX: cpu.PX++; setZ8(&cpu, cpu.PX); break;   
                    case REG_PY: cpu.PY++; setZ8(&cpu, cpu.PY); break;   
                    case REG_AX: cpu.AX++; setZ16(&cpu, cpu.AX); sync_al_ah_from_ax(&cpu); break;       
                    case REG_BX: cpu.BX++; setZ16(&cpu, cpu.BX); sync_bl_bh_from_bx(&cpu); break;    
                    case REG_CX: cpu.CX++; setZ16(&cpu, cpu.CX); break;
                    default: fprintf(stderr, "Invalid reg %u in INC\n", reg); exit(1);
                }
            } break;

            case 0x0A: {
                uint8_t reg = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                switch(reg){
                    case REG_AL: cpu.AL--; setZ8(&cpu, cpu.AL); sync_ax_from_al_ah(&cpu); break;
                    case REG_AH: cpu.AH--; setZ8(&cpu, cpu.AH); sync_ax_from_al_ah(&cpu); break;
                    case REG_BL: cpu.BL--; setZ8(&cpu, cpu.BL); sync_bx_from_bl_bh(&cpu); break;
                    case REG_BH: cpu.BH--; setZ8(&cpu, cpu.BH); sync_bx_from_bl_bh(&cpu); break;     
                    case REG_PX: cpu.PX--; setZ8(&cpu, cpu.PX); break;   
                    case REG_PY: cpu.PY--; setZ8(&cpu, cpu.PY); break;   
                    case REG_AX: cpu.AX--; setZ16(&cpu, cpu.AX); sync_al_ah_from_ax(&cpu); break;       
                    case REG_BX: cpu.BX--; setZ16(&cpu, cpu.BX); sync_bl_bh_from_bx(&cpu); break;    
                    case REG_CX: cpu.CX--; setZ16(&cpu, cpu.CX); break;
                    default: fprintf(stderr, "Invalid reg %u in DEC\n", reg); exit(1);
                }
            } break;

            case 0x0B: {
                uint16_t addr = rd16(&cpu); 
                cpu.PC = addr;
            } break;

            case 0x0C: {
                uint16_t addr = rd16(&cpu); 
                cpu.PC += 2;
                if (cpu.Z) cpu.PC = addr;
            } break;

            case 0x0D: {
                uint16_t addr = rd16(&cpu); 
                cpu.PC += 2;
                if (!cpu.Z) cpu.PC = addr;
            } break;

            case 0x0E:
                printf("%c", cpu.AL);
                break;
                
            case 0x10: {
                uint16_t stroke = cpu.BH + cpu.DS;
                if (trace){
                    printf("=== DEBUG PRINT_STR ===\n");
                    printf("BH=%02X, DS=%04X, calculated address=%04X\n", cpu.BH, cpu.DS, stroke);
                    printf("Memory dump at %04X: ", stroke);
                    for (int i = 0; i < 6; i++) {
                        uint32_t addr = (stroke + i) % MEM_SIZE;
                        uint8_t ch = mem[addr];
                        printf("[%04X]=%02X(%c) ", addr, ch, (ch >= 32 && ch < 127) ? ch : '.');
                    }
                    printf("\n");
                }
                
                int count = 0;
                while (count < 100) {
                    uint32_t addr = stroke % MEM_SIZE;
                    uint8_t ch = mem[addr];
                    if (trace) printf("Reading [%04X] = %02X -> ", addr, ch);
                    
                    if (ch == 0) {
                        if (trace) printf("END (zero terminator)\n");
                        break;
                    }
                    if (trace) printf("output '%c'\n", ch);
                    
                    printf("%c", ch);
                    stroke++;
                    count++;
                    
                    if (stroke >= MEM_SIZE) break;
                }
                if (trace) printf("=== END PRINT_STR ===\n");
            } break;

            case 0xC0: { // PUSH
                uint8_t flag = mem[(cpu.CS + cpu.PC++) % MEM_SIZE];
                uint8_t IR   = mem[(cpu.CS + cpu.PC++) % MEM_SIZE];
                uint8_t opL  = mem[(cpu.CS + cpu.PC++) % MEM_SIZE];

                if (flag == 0) {
                    if (IR == 0) {
                        mem[cpu.SP-- % MEM_SIZE] = opL;
                    } else {
                        switch (opL) {
                            case REG_AH:
                                mem[cpu.SP-- % MEM_SIZE] = cpu.AH;
                                break;
                            case REG_AL:
                                mem[cpu.SP-- % MEM_SIZE] = cpu.AL;
                                break;
                            case REG_BH:
                                mem[cpu.SP-- % MEM_SIZE] = cpu.BH;
                                break;
                            case REG_BL:
                                mem[cpu.SP-- % MEM_SIZE] = cpu.BL;
                                break;
                            case REG_PX:
                                mem[cpu.SP-- % MEM_SIZE] = cpu.PX;
                                break;
                            case REG_PY:
                                mem[cpu.SP-- % MEM_SIZE] = cpu.PY;
                                break;
                            default:
                                break;
                        }
                    }
                    if(trace){printf("PUSH - %d", mem[(cpu.SP + 1) % MEM_SIZE]);}
                } else {
                    if (IR == 0) {
                        uint8_t opH = mem[(cpu.CS + cpu.PC++) % MEM_SIZE];
                        mem[cpu.SP-- % MEM_SIZE] = opH;
                        mem[cpu.SP-- % MEM_SIZE] = opL;
                    } else {
                        switch (opL) {
                            case REG_AX: {
                                uint8_t low  = cpu.AX & 0xFF;
                                uint8_t high = cpu.AX >> 8;
                                mem[cpu.SP-- % MEM_SIZE] = high;
                                mem[cpu.SP-- % MEM_SIZE] = low;
                                break;
                            }
                            case REG_BX: {
                                uint8_t low  = cpu.BX & 0xFF;
                                uint8_t high = cpu.BX >> 8;
                                mem[cpu.SP-- % MEM_SIZE] = high;
                                mem[cpu.SP-- % MEM_SIZE] = low;
                                break;
                            }
                            case REG_CX: {
                                uint8_t low  = cpu.CX & 0xFF;
                                uint8_t high = cpu.CX >> 8;
                                mem[cpu.SP-- % MEM_SIZE] = high;
                                mem[cpu.SP-- % MEM_SIZE] = low;
                                break;
                            }
                            case REG_DS: {
                                uint8_t low  = cpu.DS & 0xFF;
                                uint8_t high = cpu.DS >> 8;
                                mem[cpu.SP-- % MEM_SIZE] = high;
                                mem[cpu.SP-- % MEM_SIZE] = low;
                                break;
                            }
                            case REG_CS: {
                                uint8_t low  = cpu.CS & 0xFF;
                                uint8_t high = cpu.CS >> 8;
                                mem[cpu.SP-- % MEM_SIZE] = high;
                                mem[cpu.SP-- % MEM_SIZE] = low;
                                break;
                            }
                            case REG_SP: {
                                uint8_t low  = cpu.SP & 0xFF;
                                uint8_t high = cpu.SP >> 8;
                                mem[cpu.SP-- % MEM_SIZE] = high;
                                mem[cpu.SP-- % MEM_SIZE] = low;
                                break;
                            }
                            case REG_PC: {
                                uint8_t low  = cpu.PC & 0xFF;
                                uint8_t high = cpu.PC >> 8;
                                mem[cpu.SP-- % MEM_SIZE] = high;
                                mem[cpu.SP-- % MEM_SIZE] = low;
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    uint16_t final_number = (final_number & 0x00FF) | ((uint16_t)mem[(cpu.SP + 2) % MEM_SIZE] << 8);
                    final_number = (final_number & 0xFF00) | mem[(cpu.SP + 1) % MEM_SIZE];
                    if(trace){printf("PUSH lowbyte- %d, highbyte- %d, final number- %d", mem[(cpu.SP + 1) % MEM_SIZE], mem[(cpu.SP + 2) % MEM_SIZE], final_number);}
                }
            } break;


            case 0xD0: {
                uint8_t IR = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t op1 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t op2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint16_t val1 = 0, val2 = 0;

                switch (op1) {
                    case REG_AL: val1 = cpu.AL; break;
                    case REG_AH: val1 = cpu.AH; break;
                    case REG_BL: val1 = cpu.BL; break;
                    case REG_BH: val1 = cpu.BH; break;
                    case REG_PX: val1 = cpu.PX; break;
                    case REG_PY: val1 = cpu.PY; break;
                    case REG_AX: val1 = cpu.AX; break;
                    case REG_BX: val1 = cpu.BX; break;
                    case REG_CX: val1 = cpu.CX; break;
                    default: val1 = op1; break;
                }
                if (IR == 1){
                    switch (op2) {
                        case REG_AL: val2 = cpu.AL; break;
                        case REG_AH: val2 = cpu.AH; break;
                        case REG_BL: val2 = cpu.BL; break;
                        case REG_BH: val2 = cpu.BH; break;
                        case REG_PX: val2 = cpu.PX; break;
                        case REG_PY: val2 = cpu.PY; break;
                        case REG_AX: val2 = cpu.AX; break;
                        case REG_BX: val2 = cpu.BX; break;
                        case REG_CX: val2 = cpu.CX; break;
                        default: val2 = op2; break;
                    }
                }
                else if(IR == 0){
                    uint8_t op3 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                    val2 = (val2 & 0x00FF) | ((uint16_t)op3 << 8);
                    val2 = (val2 & 0xFF00) | op2;
                }
                cpu.Z = (val1 == val2);

                if (trace) {
                    printf("CMP: %u == %u -> Z=%u\n", val1, val2, cpu.Z);
                }
            } break;
   
            case 0xD1: {
                uint8_t op1 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t op2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint16_t val1 = 0, val2 = 0;

                switch (op1) {
                    case REG_AL: val1 = cpu.AL; break;
                    case REG_AH: val1 = cpu.AH; break;
                    case REG_BL: val1 = cpu.BL; break;
                    case REG_BH: val1 = cpu.BH; break;
                    case REG_PX: val1 = cpu.PX; break;
                    case REG_PY: val1 = cpu.PY; break;
                    case REG_AX: val1 = cpu.AX; break;
                    case REG_BX: val1 = cpu.BX; break;
                    case REG_CX: val1 = cpu.CX; break;
                    default: val1 = op1; break;
                }

                switch (op2) {
                    case REG_AL: val2 = cpu.AL; break;
                    case REG_AH: val2 = cpu.AH; break;
                    case REG_BL: val2 = cpu.BL; break;
                    case REG_BH: val2 = cpu.BH; break;
                    case REG_PX: val2 = cpu.PX; break;
                    case REG_PY: val2 = cpu.PY; break;
                    case REG_AX: val2 = cpu.AX; break;
                    case REG_BX: val2 = cpu.BX; break;
                    case REG_CX: val2 = cpu.CX; break;
                    default: val2 = op2; break;
                }

                cpu.Z = (val1 < val2);

                if (trace) {
                    printf("%u < %u -> Z=%u\n", val1, val2, cpu.Z);
                }
            } break;

            case 0xD2: {
                uint8_t op1 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint8_t op2 = mem[cpu.PC++ % MEM_SIZE + cpu.CS];
                uint16_t val1 = 0, val2 = 0;

                switch (op1) {
                    case REG_AL: val1 = cpu.AL; break;
                    case REG_AH: val1 = cpu.AH; break;
                    case REG_BL: val1 = cpu.BL; break;
                    case REG_BH: val1 = cpu.BH; break;
                    case REG_PX: val1 = cpu.PX; break;
                    case REG_PY: val1 = cpu.PY; break;
                    case REG_AX: val1 = cpu.AX; break;
                    case REG_BX: val1 = cpu.BX; break;
                    case REG_CX: val1 = cpu.CX; break;
                    default: val1 = op1; break;
                }

                switch (op2) {
                    case REG_AL: val2 = cpu.AL; break;
                    case REG_AH: val2 = cpu.AH; break;
                    case REG_BL: val2 = cpu.BL; break;
                    case REG_BH: val2 = cpu.BH; break;
                    case REG_PX: val2 = cpu.PX; break;
                    case REG_PY: val2 = cpu.PY; break;
                    case REG_AX: val2 = cpu.AX; break;
                    case REG_BX: val2 = cpu.BX; break;
                    case REG_CX: val2 = cpu.CX; break;
                    default: val2 = op2; break;
                }

                cpu.Z = (val1 > val2);

                if (trace) {
                    printf("%u > %u -> Z=%u\n", val1, val2, cpu.Z);
                }
            } break;
            
            case 0xF9: {
                int ch = getch();
                cpu.AL = ch;
                sync_ax_from_al_ah(&cpu);
            } break;
            
            case 0xFF:
                if (trace) printf("HALT\n");
                return 0;
            
            default:
                fprintf(stderr, "Illegal opcode %02X at %04X\n", op, (cpu.PC-1)&0xFFFF);
                return 1;
        }
    }
}