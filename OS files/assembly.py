# Copyright 2025 SWAGNER
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
# limitations under the License.
import struct
import threading 
import os
import time
import sys 

mem_for_variable = {}
PC_mem = 0
PC = 0
mem = [0] * 65536
valid_save = True
compile_mode = False
withfile = False

def sync_ax_to_parts():
    global registers
    ax_value = registers["AX"]
    registers["AL"] = ax_value & 0xFF
    registers["AH"] = (ax_value >> 8) & 0xFF

def sync_parts_to_ax():
    global registers
    registers["AX"] = (registers["AH"] << 8) | registers["AL"]

def sync_bx_to_parts():
    global registers
    bx_value = registers["BX"]
    registers["BL"] = bx_value & 0xFF
    registers["BH"] = (bx_value >> 8) & 0xFF

def sync_parts_to_bx():
    global registers
    registers["BX"] = (registers["BH"] << 8) | registers["BL"]

do_start = True
new_project = False

commands = []
labels = {}

def compile_all():
    global commands, labels, compile_mode
    
    print("Начало компиляции...")
    
    # Первый проход - сбор меток
    current_address = 0
    for command in commands:
        if command.endswith(':'):
            label_name = command[:-1]
            labels[label_name] = current_address
            print(f"Метка '{label_name}' по адресу {current_address}")
        else:
            cmd_parts = command.split()
            if cmd_parts and cmd_parts[0] != 'compile':  # Пропускаем команду compile
                opcode = cmd_parts[0]
                # Подсчет размера команды
                if opcode in ['mov', 'store', 'load', 'setv']:
                    current_address += 4
                elif opcode in ['jmp', 'jz', 'jnz']:
                    current_address += 3
                elif opcode in ['add', 'sub', 'comp']:
                    current_address += 3
                elif opcode in ['inc', 'dec']:
                    current_address += 2
                elif opcode in ['print', 'input', 'f', 'stop', 'printstr', 'render', 'init']:
                    current_address += 1

    # Второй проход - генерация кода
    binaryd.seek(0)
    binaryd.truncate(0)
    
    current_address = 0
    for command in commands:
        if command.endswith(':'):
            continue  # Пропускаем метки
            
        cmd_parts = command.split()
        if not cmd_parts or cmd_parts[0] == 'compile':
            continue  # Пропускаем пустые команды и команду compile
            
        opcode = cmd_parts[0]
        operand1 = cmd_parts[1] if len(cmd_parts) > 1 else None
        operand2 = cmd_parts[2] if len(cmd_parts) > 2 else None
        operand3 = cmd_parts[3] if len(cmd_parts) > 3 else None

        print(f"Компиляция: {command} по адресу {current_address}")
        
        # Компиляция команд
        
        if opcode == 'mov':
            if operand2 in mem_for_variable:
                operand2 = mem_for_variable[operand2][0][1]
            compile_mov(operand1, operand2)
            current_address += 4
        elif opcode == 'printstr':
            compile_print_str()
            current_address += 1  
        elif opcode == 'setv':
            compile_set_VGA(operand1, operand2, operand3)      
            current_address += 4
        elif opcode == 'init':
            compile_init()
            current_address += 1
        elif opcode == 'render':
            compile_render()
            current_address += 1        
        elif opcode == 'jmp':
            compile_jump('jmp', operand1)
            current_address += 3
        elif opcode == 'jz':
            compile_jump('jz', operand1)
            current_address += 3
        elif opcode == 'jnz':
            compile_jump('jnz', operand1)
            current_address += 3
        elif opcode == 'add':
            compile_add(operand1, operand2)
            current_address += 3
        elif opcode == 'sub':
            compile_sub(operand1, operand2)
            current_address += 3
        elif opcode == 'inc':
            compile_inc(operand1)
            current_address += 2
        elif opcode == 'dec':
            compile_dec(operand1)
            current_address += 2
        elif opcode == 'store':
            compile_store(operand1, operand2)
            current_address += 4
        elif opcode == 'load':
            compile_load(operand1, operand2)
            current_address += 4
        elif opcode == 'comp':
            compile_comp(operand1, operand2)
            current_address += 3
        elif opcode == '<':
            compile_less(operand1, operand2)
            current_address += 4    
        elif opcode == '>':
            compile_greater(operand1, operand2)
            current_address += 4        
        elif opcode == 'print' or opcode == 'printstr':
            compile_print()
            current_address += 1
        elif opcode == 'input':
            compile_input()
            current_address += 1
        elif opcode == 'f' or opcode == 'stop':
            compile_stop()
            current_address += 1
    
    compile_mode = False
    valid_commands = [cmd for cmd in commands if not cmd.endswith(':') and not cmd.startswith('compile')]
    print("Компиляция завершена!")
    print(f"Всего команд: {len(valid_commands)}")
    print(f"Метки: {labels}")

def compile_mov(op1, op2):
    reg1_code = valid_of_reg(op1)
    if reg1_code == -1:
        print(f"Ошибка компиляции: неверный регистр {op1}")
        return
    
    reg1_name = table_of_reg[reg1_code].upper()
    flag = 1 if reg1_name in ["AX", "BX", "DS", "CS"] else 0
    
    try:
        binaryd.write(struct.pack("<B", 0x20))
        binaryd.write(struct.pack("<B", flag))
        binaryd.write(struct.pack("<B", reg1_code))
        
        # Преобразуем в строку, если это число
        if isinstance(op2, int):
            op2 = str(op2)
            value = parse_operand(op2)
            value -= registers["DS"]
        else:
            value = parse_number(op2)

        if flag == 1:
            # 16-битные регистры
            binaryd.write(struct.pack("<H", value))
        else:
            # 8-битные регистры - проверяем переполнение
            if valid_of_reg(value) != -1:
                value = valid_of_reg(value)
            if value > 255:
                print(f"Ошибка: значение {value} слишком большое для 8-битного регистра {reg1_name}")
                return
            binaryd.write(struct.pack("<B", value))
            
    except Exception as e:
        print(f"Ошибка компиляции mov: {e}")

def compile_jump(opcode, operand):
    opcodes = {
        'jmp': 0x0B,
        'jz': 0x0C, 
        'jnz': 0x0D
    }
    
    if opcode not in opcodes:
        return
        
    try:
        binaryd.write(struct.pack("<B", opcodes[opcode]))
        address = resolve_label(operand)
        binaryd.write(struct.pack("<H", address))
        print(f"  Переход на '{operand}' -> адрес {address}")
    except Exception as e:
        print(f"Ошибка компиляции {opcode}: {e}")

def compile_add(op1, op2):
    try:
        binaryd.write(struct.pack("<B", 0x08))
        binaryd.write(struct.pack("<B", valid_of_reg(op1)))
        binaryd.write(struct.pack("<B", parse_operand(op2)))
    except Exception as e:
        print(f"Ошибка компиляции add: {e}")

def compile_sub(op1, op2):
    try:
        binaryd.write(struct.pack("<B", 0x07))
        binaryd.write(struct.pack("<B", valid_of_reg(op1)))
        binaryd.write(struct.pack("<B", parse_operand(op2)))
    except Exception as e:
        print(f"Ошибка компиляции sub: {e}")

def compile_inc(op1):
    try:
        binaryd.write(struct.pack("<B", 0x09))
        binaryd.write(struct.pack("<B", valid_of_reg(op1)))
    except Exception as e:
        print(f"Ошибка компиляции inc: {e}")

def compile_dec(op1):
    try:
        binaryd.write(struct.pack("<B", 0x0A))
        binaryd.write(struct.pack("<B", valid_of_reg(op1)))
    except Exception as e:
        print(f"Ошибка компиляции dec: {e}")

def compile_init():
    try:
        binaryd.write(struct.pack("<B", 0x32))
    except Exception as e:
        print(f"Ошибка компиляции dec: {e}")

def compile_render():
    try:
        binaryd.write(struct.pack("<B", 0x31))
    except Exception as e:
        print(f"Ошибка компиляции dec: {e}")

def compile_store(op1, op2):
    try:
        reg_code = valid_of_reg(op2)
        if reg_code >= 0:
            print(f"DEBUG COMPILE: STORE [{op1}], регистр {table_of_reg[reg_code]}")
            print(f"  Код: 05 {reg_code:02X} {int(op1):04X}h")
            binaryd.write(struct.pack("<B", 0x05))
            binaryd.write(struct.pack("<B", reg_code))
            binaryd.write(struct.pack("<H", int(op1)))
        else:
            print(f"DEBUG COMPILE: STORE [{op1}], значение {op2}")
            print(f"  Код: 05 00 {int(op1):04X}h")
            binaryd.write(struct.pack("<B", 0x05))
            binaryd.write(struct.pack("<B", int(op2)))  # 0 означает "значение"
            binaryd.write(struct.pack("<H", int(op1)))
    except Exception as e:
        print(f"Ошибка компиляции store: {e}")

def compile_load(op1, op2):
    try:
        reg_code = valid_of_reg(op1)
        if reg_code >= 0:
            binaryd.write(struct.pack("<B", 0x04))
            binaryd.write(struct.pack("<B", reg_code))
            binaryd.write(struct.pack("<H", int(op2)))
    except Exception as e:
        print(f"Ошибка компиляции load: {e}")

def compile_comp(op1, op2):
    try:
        binaryd.write(struct.pack("<B", 0xD0))
        binaryd.write(struct.pack("<B", valid_of_reg(op1)))
        binaryd.write(struct.pack("<B", valid_of_reg(op2)))
    except Exception as e:
        print(f"Ошибка компиляции comp: {e}")

def compile_less(op1, op2):
    try:
        binaryd.write(struct.pack("<B", 0xD1))
        binaryd.write(struct.pack("<B", valid_of_reg(op1)))
        binaryd.write(struct.pack("<B", valid_of_reg(op2)))
    except Exception as e:
        print(f"Ошибка компиляции comp: {e}")

def compile_greater(op1, op2):
    try:
        binaryd.write(struct.pack("<B", 0xD2))
        binaryd.write(struct.pack("<B", valid_of_reg(op1)))
        binaryd.write(struct.pack("<B", valid_of_reg(op2)))
    except Exception as e:
        print(f"Ошибка компиляции comp: {e}")

def compile_print():
    try:
        binaryd.write(struct.pack("<B", 0x0E))
    except Exception as e:
        print(f"Ошибка компиляции print: {e}")

def compile_input():
    try:
        binaryd.write(struct.pack("<B", 0xF9))
    except Exception as e:
        print(f"Ошибка компиляции input: {e}")

def compile_stop():
    try:
        binaryd.write(struct.pack("<B", 0xFF))
    except Exception as e:
        print(f"Ошибка компиляции stop: {e}")
def compile_print_str():
    try:
        binaryd.write(struct.pack("<B", 0x10))  
    except Exception as e:
        print(f"Ошибка компиляции print_str: {e}")

def compile_set_VGA(x,y,color):
    try:
        binaryd.write(struct.pack("<B", 0x30))  
    except Exception as e:
        print(f"Ошибка компиляции print_str: {e}")
    if isinstance(x, str):
        try:
            x = int(x)
        except:
            if x in registers:
                x = registers[x]
            else:
                print("Ошибка компиляции set_VGA, регистр X не найден")
                return
    if isinstance(y, str):
        try:
            y = int(y)
        except:
            if y in registers:
                y = registers[x]
            else:
                print("Ошибка компиляции set_VGA, регистр Y не найден")
                return
    if isinstance(color, str):
        try:
            color = int(color)
        except:
            if color in registers:
                color = registers[color]
            else:
                print("Ошибка компиляции set_VGA, регистр color не найден")    
                return    
    binaryd.write(struct.pack("<B", x))      
    binaryd.write(struct.pack("<B", y))        
    binaryd.write(struct.pack("<B", color))   

def parse_operand(operand):
    if valid_of_reg(operand)!= -1:
        return valid_of_reg(operand)
    if operand is None:
        return 0
    if operand in labels:
        return labels[operand]
    if operand in mem_for_variable:
        # Для компиляции возвращаем абсолютный адрес
        # Вычитание DS будет происходить во время выполнения
        return mem_for_variable[operand][0][1]
    try:
        if operand.startswith("0x") or operand.startswith("0X"):
            return int(operand, 16)
        else:
            return int(operand)
    except ValueError:
        reg_code = valid_of_reg(operand)
        if reg_code != -1:
            return reg_code
        else:
            print(f"Неизвестный операнд: {operand}")
            return 0

def resolve_label(label_name):
    if label_name in labels:
        return labels[label_name]
    else:
        print(f"Неизвестная метка: {label_name}")
        return 0

def valid_of_reg(reg):
    global table_of_reg
    try:
        reg = int(reg)
        if reg >= 0 and reg < 8:
            return reg
        else:
            return -1
    except:   
        if reg in table_of_reg:
            return table_of_reg[reg]
        else:
            return -1

def mov(op1, op2):
    global registers, table_of_reg
    
    reg1_code = valid_of_reg(op1)
    
    if reg1_code == -1:
        print(f"Ошибка: неверный регистр {op1}")
        return 1
    
    reg1_name = table_of_reg[reg1_code].upper()
    
    reg2_code = valid_of_reg(op2)
    if reg2_code != -1:
        reg2_name = table_of_reg[reg2_code].upper()
        registers[reg1_name] = registers[reg2_name]
        print(f"{reg1_name} = {registers[reg1_name]}")
    else:
        try:
            value = int(op2)
            registers[reg1_name] = value
            print(f"{reg1_name} = {value}")
        except:
            if op2 in mem_for_variable:
                registers[reg1_name] = mem_for_variable[op2][0][1]-registers["DS"]
                print(f"{reg1_name} = {mem_for_variable[op2][0][1]-registers["DS"]}")
                return 0
            print(f"Ошибка: неверный второй операнд {op2}(если вы указали переменную данная ошибка дефолт)")
            return 1
    
    if reg1_code == 4:
        sync_ax_to_parts()
    elif reg1_code == 0 or reg1_code == 1:
        sync_parts_to_ax()
    elif reg1_code == 5:
        sync_bx_to_parts()
    elif reg1_code == 2 or reg1_code == 3:
        sync_parts_to_bx()
    
    return 0 


table_of_reg = {
    # --- 8-битные регистры ---
    "AL": 0, "AH": 1, "BL": 2, "BH": 3,

    # --- 16-битные и спецрегистры ---
    "AX": 4, "BX": 5, "DS": 6, "CS": 7, "CX": 8, "PX": 9, "PY": 10,

    # --- нижний регистр для удобства ---
    "al": 0, "ah": 1, "bl": 2, "bh": 3,
    "ax": 4, "bx": 5, "ds": 6, "cs": 7, "cx": 8, "px": 9, "py": 10,

    # --- обратное отображение (индекс -> имя) ---
    0: "AL", 1: "AH", 2: "BL", 3: "BH",
    4: "AX", 5: "BX", 6: "DS", 7: "CS",
    8: "CX", 9: "PX", 10: "PY"
}


registers = {
    "AL": 0, "AH": 0, "BL": 0, "BH": 0, "AX": 0, "BX": 0, "DS": 4096, "CS": 0, "CX": 0, "PX": 0, "PY": 0
}

def save_out():
    try:
        temp = open(file, "wb+")
        save = input("откуда сохранить?(название через .bin):")
        try:
            code = open(save, "rb")
            data = code.read()
            temp.write(data)
            code.close()
            return 0
        except:
            print("такого файла нет..")
            return 1
    except:
        return 1    

def inc(op1):
    global table_of_reg, registers
    reg_code = valid_of_reg(op1)
    if reg_code == -1:
        print("Ошибка: неверный регистр")
        return
    
    reg_name = table_of_reg[reg_code].upper()
    registers[reg_name] += 1
    
    if reg_name == "AX":
        sync_ax_to_parts()
    elif reg_name == "BX":
        sync_bx_to_parts()
    elif reg_name == "AL" or reg_name == "AH":
        sync_parts_to_ax()
    elif reg_name == "BL" or reg_name == "BH":
        sync_parts_to_bx()
    return

def dec(op1):
    global table_of_reg, registers
    reg_code = valid_of_reg(op1)
    if reg_code == -1:
        print("Ошибка: неверный регистр")
        return
    
    reg_name = table_of_reg[reg_code].upper()
    registers[reg_name] -= 1
    
    if reg_name == "AX":
        sync_ax_to_parts()
    elif reg_name == "BX":
        sync_bx_to_parts()
    elif reg_name == "AL" or reg_name == "AH":
        sync_parts_to_ax()
    elif reg_name == "BL" or reg_name == "BH":
        sync_parts_to_bx()
    return
def add(reg1, reg2):
    global table_of_reg, registers
    reg1_code = valid_of_reg(reg1)
    reg2_code = valid_of_reg(reg2)
    
    if reg1_code == -1 or reg2_code == -1:
        print("Ошибка: неверный регистр")
        return
    
    reg1_name = table_of_reg[reg1_code].upper()
    
    if reg2_code != -1:
        reg2_name = table_of_reg[reg2_code].upper()
        registers[reg1_name] += registers[reg2_name]
    else:
        registers[reg1_name] += int(reg2)
    
    if reg1_name == "AX":
        sync_ax_to_parts()
    elif reg1_name == "BX":
        sync_bx_to_parts()
    elif reg1_name == "AL" or reg1_name == "AH":
        sync_parts_to_ax()
    elif reg1_name == "BL" or reg1_name == "BH":
        sync_parts_to_bx()
    return

def sub(reg1, reg2):
    global table_of_reg, registers
    reg1_code = valid_of_reg(reg1)
    
    if reg1_code == -1:
        print("Ошибка: неверный первый регистр")
        return
    
    reg1_name = table_of_reg[reg1_code].upper()
    reg2_code = valid_of_reg(reg2)
    
    if reg2_code != -1:
        reg2_name = table_of_reg[reg2_code].upper()
        registers[reg1_name] -= registers[reg2_name]  
        print(f"{reg1_name} = {reg1_name} - {reg2_name} = {registers[reg1_name]}")
    else:
        try:
            value = int(reg2)
            registers[reg1_name] -= value  
            print(f"{reg1_name} = {reg1_name} - {value} = {registers[reg1_name]}")
        except:
            print("Ошибка: неверный второй операнд")
            return
    
    if reg1_name == "AX":
        sync_ax_to_parts()
    elif reg1_name == "BX":
        sync_bx_to_parts()
    elif reg1_name == "AL" or reg1_name == "AH":
        sync_parts_to_ax()
    elif reg1_name == "BL" or reg1_name == "BH":
        sync_parts_to_bx()
    return

def parse_number(s):
    if s is None:
        return 0
    try:
        if s.startswith("0x") or s.startswith("0X"):
            return int(s, 16)
        else:
            return int(s)
    except ValueError:
        return s

def switch(incode, operrand=None, operrand2=None, operrand3=None):
    global file, do_start, valid_save, mem, PC
    global binaryd, PC_mem, mem_for_variable, commands, compile_mode
    
    # Обработка меток
    if incode.endswith(':'):
        label_name = incode[:-1]
        mem_for_variable[label_name] = [["label", PC]]
        commands.append(incode)
        return
        
    # Список валидных команд для компиляции (только те, что генерируют код)
    valid_compile_commands = [
        'store', 'load', 'mov', 'jmp', 'jz', 'jnz', 'add', 'sub', 
        'inc', 'dec', 'print', 'comp', 'input', 'f', 'stop', 'printstr', '<', '>', 'init', 'render', 'setv'
    ]
    
    # Проверка валидности команды (только для компилируемых команд)
    if incode not in valid_compile_commands and incode not in mem_for_variable and operrand != "=":
        # Это служебная команда интерпретатора, не добавляем в список для компиляции
        pass
    else:
        # Добавление команды в список для компиляции
        if operrand and operrand2 and operrand3: #если все не пустые
            commands.append(f"{incode} {operrand} {operrand2} {operrand3}")
        elif operrand and operrand2:  # если оба не пустые/не None
            commands.append(f"{incode} {operrand} {operrand2}")
        elif operrand:  # если только operrand не пустой
            commands.append(f"{incode} {operrand}")
        else:  # если оба пустые
            commands.append(incode)
        
    # Компиляция
    if incode == "compile":
        valid_save = True
        compile_mode = True
        compile_all()
        return
        
    elif incode == "com":
        print("Команды:", commands)
        print("Метки:", labels)
    elif incode == "help":
        print("""store — сохраняет значение регистра в память по адресу
load — загружает значение из памяти в регистр
clear — очищает файлы программы
q — завершает программу
f / stop — конец программы
mov — перемещение данных(числа загружаются только больше 7!!! т.к. до 7 это номера регистров)
compile - выполняет двухэтапную компиляцию
jmp — безусловный переход
jz — переход при нуле
jnz — переход при ненуле
add — сложение
sub — вычитание
inc — инкремент
dec — декремент
print — вывод
printstr - вывод строки, адрес начала строки хранится в регистре bh
input — ввод
com - показать список команд для компиляции
---: - файл сохранен
***: - файл не сохранен""")
        return    
    elif incode == "printstr":
        valid_save = False
        return
    elif incode == "setv":
        valid_save = False
        return
    elif incode == "<":
        valid_save = False
        return
    elif incode == ">":
        valid_save = False
        return
    elif incode == "render":
        valid_save = False
        return
    elif incode == "init":
        valid_save = False
        return
    elif incode == "store":
        valid_save = False
        try:
            a = valid_of_reg(operrand2)
            if a >= 0:
                mem[int(operrand)+registers["DS"]] = registers[table_of_reg[a]]
                PC_mem += 1
                PC += 4    
            else:
                try: 
                    operrand2 = int(operrand2) 
                    mem[int(operrand)+registers["DS"]] = operrand2
                    PC += 4    
                    PC_mem += 1
                except:
                    print("введен неверный регистр")
        except:
            print("ошибка записи!")    
        return
    elif incode == "load":
        valid_save = False
        try:
            a = valid_of_reg(operrand)
            if a >= 0:
                registers[table_of_reg[a]] = mem[int(operrand2)+registers["DS"]]
                PC_mem += 1
            else:
                print("ошибка регистра!")
            PC += 4    
        except:
            print("ошибка записи!")    
        return
    elif incode == "reg":
        print("=== РЕГИСТРЫ ===")
        print(f"AX: {registers['AX']} (0x{registers['AX']:04X})")
        print(f"  AL: {registers['AL']} (0x{registers['AL']:02X}), AH: {registers['AH']} (0x{registers['AH']:02X})")
        print(f"BX: {registers['BX']} (0x{registers['BX']:04X})")
        print(f"  BL: {registers['BL']} (0x{registers['BL']:02X}), BH: {registers['BH']} (0x{registers['BH']:02X})")
        print(f"DS: {registers['DS']}, CS: {registers['CS']}")
        return
    elif incode == "clear":
        valid_save = False
        print("уверен?")
        temp = input("y/n:")
        if temp == "y" or temp == "Y" or temp == "yes":
            binaryd.truncate(0)
            commands.clear()
            labels.clear()
            print("удалено")
        else:
            print("отменено")
        return     
    elif incode == "mem":
        try:
            print(mem[int(operrand)+registers["DS"]])    
        except:
            print("указывайте правильную память")    
        return
    elif incode == "q":
        binaryd.seek(0, 2)
        print("Thank you for using Letos;)")
        do_start = False    
    elif incode == "f" or incode == "stop":
        valid_save = False
        return
    elif incode == "mov":
        valid_save = False
        mov(operrand, operrand2)
        return         
    elif incode == "jmp":
        valid_save = False
        return
    elif incode == "jz":
        valid_save = False
        return
    elif incode == "jnz":
        valid_save = False
        return
    elif incode == "add":
        valid_save = False
        add(operrand, operrand2)
        return
    elif incode == "sub":
        valid_save = False
        sub(operrand, operrand2)
        return
    elif incode == "inc":
        valid_save = False
        inc(operrand)
        return
    elif incode == "dec":
        valid_save = False
        dec(operrand)
        return
    elif incode == "print":
        valid_save = False
        return           
    elif incode == "comp":
        valid_save = False
        return
    elif incode == "PC":
        PC_mem = int(operrand)    
        return
    elif incode == "input":   
        valid_save = False
        return
    elif incode in mem_for_variable:
        try:
            operrand = int(operrand)
        except:
            pass   
        if valid_of_reg(operrand2) >= 0:  
            if isinstance(operrand, str):
                if operrand.upper() in registers:
                    try:
                        switch("mov", operrand2, mem_for_variable[incode][registers[operrand.upper()]][0])
                    except:
                        print("Ошибка загрузки символа в регистр!")   
                        return -1        
            else:
                try:
                    switch("mov", operrand2, mem_for_variable[incode][operrand][0])
                except:
                    print("Ошибка загрузки символа в регистр!")   
                    return -1        
        else:
            print("неизвестный регистр, будет использоваться регистр al")    
            if isinstance(operrand, str):
                try:
                    switch("mov", operrand2, mem_for_variable[incode][registers[operrand.upper()]][0])
                except:
                    print("Ошибка загрузки символа в регистр!")   
                    return -1     
            else:
                try:
                    switch("mov", "al", mem_for_variable[incode][0][operrand])
                except:
                    print("Ошибка загрузки символа в регистр!")    
                    return -1        
        return 0    
    elif incode == "var":
        print(mem_for_variable)   
        return     
    else:
        if operrand == "=":
            variable(incode, operrand2, operrand)     
        else:
            # Служебные команды интерпретатора (var, si, so, b, r, cc, разослать и т.д.)
            # не добавляются в commands и не считаются ошибкой
            pass
        return

def variable(name, data, oper):
    global registers, PC_mem, mem_for_variable

    # Проверка на None
    if data is None:
        print(f"Ошибка: данные для переменной '{name}' не указаны")
        return

    try:
        data = int(data)
    except:
        pass

    if oper == "=":
        if isinstance(data, str) or (data is not None and data > 255):
            data = str(data)
            if len(data) == 1:
                value = ord(data)
                if name in mem_for_variable:
                    PC_mem = mem_for_variable[name][1]
                else:
                    PC_mem += 1
                
                # СНАЧАЛА устанавливаем значение в AL, ПОТОМ сохраняем
                switch("mov", "al", str(value))  # явно преобразуем в строку
                switch("store", str(PC_mem), "al")  # явно преобразуем в строку
                mem_for_variable[name] = [value, PC_mem-1+registers["DS"]]
                return
            else:
                if name not in mem_for_variable:
                    mem_for_variable[name] = []

                for ch in data:
                    value = ord(ch)
                    mem_for_variable[name].append([value, PC_mem+registers["DS"]])
                    # СНАЧАЛА устанавливаем значение в AL, ПОТОМ сохраняем
                    switch("mov", "al", str(value))
                    switch("store", str(PC_mem), "al")
                
                # Добавляем нулевой терминатор
                switch("sub", "al", "al")
                switch("store", str(PC_mem), "al")
                mem_for_variable[name].append([0, PC_mem-1+registers["DS"]])
                
                return
        else:
            if name in mem_for_variable:
                PC_mem = mem_for_variable[name][1]
            else:
                PC_mem += 1
            
            # Для числовых значений
            switch("mov", "al", str(data))
            switch("store", str(PC_mem), "al")
            mem_for_variable[name] = [int(data), PC_mem+registers["DS"]]

    return

def start(ans=None):
    global valid_save, do_start, withfile
    
    if ans is None:
        if valid_save:
            ans = input("---:")
        else:
            ans = input("***:")
    
    parts = ans.split()
    command = parts[0] if parts else ""
    
    if command == "q":
        switch("compile")
        do_start = False
        return
        
    if len(parts) == 1:
        switch(command)
    elif len(parts) == 2:
        opperand = parts[1]
        switch(command, opperand)   
    elif len(parts) == 3:
        opperand = parts[1]
        opperand2 = parts[2]    
        switch(command, opperand, opperand2)   
    elif len(parts) == 4:
        opperand = parts[1]
        opperand2 = parts[2]    
        operrand3 = parts[3]    
        switch(command, opperand, opperand2, operrand3)       
           
    # Не запускаем цикл при работе с файлом
    if do_start and not withfile:
        while do_start:
            start()

if len(sys.argv) > 1:
    new_project = True
    withfile = True
    comandfile = sys.argv[1]
    
    # Правильное определение имени бинарного файла
    binfile = os.path.splitext(comandfile)[0] + ".bin"
    
    try:
        # Открываем или создаем бинарный файл
        if os.path.exists(binfile):
            binaryd = open(binfile, "r+b")
        else:
            binaryd = open(binfile, "wb+")
            
        # Читаем команды из файла
        with open(comandfile, "r", encoding="utf-8") as code:
            for line_num, line in enumerate(code, 1):
                line = line.strip()
                if not line or line.startswith(';') or line.startswith('#'):
                    continue
                print(f"Обрабатываем строку {line_num}: {line}")
                start(line)
            
            # Автоматически компилируем и завершаем после обработки всех команд
            switch("compile")
            print("Компиляция завершена автоматически после обработки файла")
        
        binaryd.close()
        do_start = False
        
    except Exception as e:
        print(f"Ошибка загрузки файла: {e}")
        do_start = False
else:    
    print(""">Booting compiler...
    ================================
    L      EEEEE  TTTTT   OOO   SSSS
    L      E        T    O   O  S
    L      EEE      T    O   O  SSS
    L      E        T    O   O     S
    LLLLL  EEEEE    T     OOO   SSSS version 0.0.4!
    ================================""")
    file = input("напишите навзание файла для открытия:")
    print("Trying to create file..")
    if file == "q":
            do_start = False
            print("Failed")
    else:           
        try:
            if file.endswith(".bin"):
                binaryd = open(file, "r+b")
            else:
                file = (file + ".bin")    
                binaryd = open(file, "r+b")
            print("Complite!")    
        except:
            print("Failed")
            if (input("такого файла нет, хотите создать?y/n:")) == "y":
                if file.endswith(".bin"):
                    binaryd = open(file, "wb+")
                else:
                    file = (file + ".bin")    
                    binaryd = open(file, "wb+")
                print("Complite!")    
            else:        
                print("Creating temp file..")
                file = "temp.bin"
                binaryd = open("temp.bin", "wb+")
                print("Complite!")
        print("Work with .swg file..")        
        new_file = input("вы хотите очистить данные(все данные с файла .swg сотрутся)y/n:")   
        if new_file == "y" or new_file == "yes":
            new_project = True   
        temp_file = file[:-4]    
        swgfile = open(temp_file+".swg", "+a")
        print("вы в файле:", file)  
        print("Ready for work!")    
        print("введите help для ознакомления")
    
    # Убрана работа с .swg файлом
    try:
        PC_mem = max(addr for values in mem_for_variable.values() for _, addr in values)
    except:
        pass

if do_start:
    start()