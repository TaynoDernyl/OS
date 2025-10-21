import struct
import keyboard
import threading 
import os
import time
import sys 
#===================================
ascii_table = {
    0: 'NUL', 1: 'SOH', 2: 'STX', 3: 'ETX', 4: 'EOT', 5: 'ENQ', 6: 'ACK', 7: 'BEL',
    8: 'BS', 9: 'TAB', 10: 'LF', 11: 'VT', 12: 'FF', 13: 'CR', 14: 'SO', 15: 'SI',
    16: 'DLE', 17: 'DC1', 18: 'DC2', 19: 'DC3', 20: 'DC4', 21: 'NAK', 22: 'SYN', 23: 'ETB',
    24: 'CAN', 25: 'EM', 26: 'SUB', 27: 'ESC', 28: 'FS', 29: 'GS', 30: 'RS', 31: 'US',
    32: ' ', 33: '!', 34: '"', 35: '#', 36: '$', 37: '%', 38: '&', 39: "'",
    40: '(', 41: ')', 42: '*', 43: '+', 44: ',', 45: '-', 46: '.', 47: '/',
    48: '0', 49: '1', 50: '2', 51: '3', 52: '4', 53: '5', 54: '6', 55: '7',
    56: '8', 57: '9', 58: ':', 59: ';', 60: '<', 61: '=', 62: '>', 63: '?',
    64: '@', 65: 'A', 66: 'B', 67: 'C', 68: 'D', 69: 'E', 70: 'F', 71: 'G',
    72: 'H', 73: 'I', 74: 'J', 75: 'K', 76: 'L', 77: 'M', 78: 'N', 79: 'O',
    80: 'P', 81: 'Q', 82: 'R', 83: 'S', 84: 'T', 85: 'U', 86: 'V', 87: 'W',
    88: 'X', 89: 'Y', 90: 'Z', 91: '[', 92: '\\', 93: ']', 94: '^', 95: '_',
    96: '`', 97: 'a', 98: 'b', 99: 'c', 100: 'd', 101: 'e', 102: 'f', 103: 'g',
    104: 'h', 105: 'i', 106: 'j', 107: 'k', 108: 'l', 109: 'm', 110: 'n', 111: 'o',
    112: 'p', 113: 'q', 114: 'r', 115: 's', 116: 't', 117: 'u', 118: 'v', 119: 'w',
    120: 'x', 121: 'y', 122: 'z', 123: '{', 124: '|', 125: '}', 126: '~', 127: 'DEL'
}
#===================================
mem_for_variable = {} #наши переменные
PC_mem = 0
mem = [0] * 65536
valid_save = True
#===================================
def sync_ax_to_parts():
    """Синхронизирует AX -> AL и AH"""
    global registers
    ax_value = registers["AX"]
    registers["AL"] = ax_value & 0xFF          # Младший байт
    registers["AH"] = (ax_value >> 8) & 0xFF   # Старший байт

def sync_parts_to_ax():
    """Синхронизирует AL и AH -> AX"""
    global registers
    registers["AX"] = (registers["AH"] << 8) | registers["AL"]

def sync_bx_to_parts():
    """Синхронизирует BX -> BL и BH"""
    global registers
    bx_value = registers["BX"]
    registers["BL"] = bx_value & 0xFF          # Младший байт
    registers["BH"] = (bx_value >> 8) & 0xFF   # Старший байт

def sync_parts_to_bx():
    """Синхронизирует BL и BH -> BX"""
    global registers
    registers["BX"] = (registers["BH"] << 8) | registers["BL"]
#===================================
def load_variables_from_file(comandfile):
    global mem_for_variable, mem
    with open(comandfile, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or ":" not in line:
                continue  # пропускаем пустые строки и мусор

            name, rest = line.split(":", 1)
            name = name.strip()

            pairs = []
            # ищем все пары вида (value=число, addr=число)
            import re
            matches = re.findall(r"\(value=(\d+), addr=(\d+)\)", rest)
            for val, addr in matches:
                pairs.append([int(val), int(addr)])
                mem[int(addr)] = int(val)
                if int(addr) > len(mem):
                    print("адрес больше памяти")
                print(mem[int(addr)])    
            mem_for_variable[name] = pairs

    return mem_for_variable
#===================================
do_start = True
new_project = False
#===================================
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
#===================================
def mov(op1, op2):
    global registers, table_of_reg
    
    reg1_code = valid_of_reg(op1)
    
    if reg1_code == -1:
        print(f"Ошибка: неверный регистр {op1}")
        return 1
    
    # Получаем имя регистра в ВЕРХНЕМ регистре
    reg1_name = table_of_reg[reg1_code].upper()
    
    # Определяем размер операции
    if reg1_name in ["AX", "BX", "DS", "CS"]:
        flag = 1
    else:
        flag = 0
    
    # Обновляем значение в registers
    reg2_code = valid_of_reg(op2)
    if reg2_code != -1:  # op2 - регистр
        # Получаем имя второго регистра
        reg2_name = table_of_reg[reg2_code].upper()
        registers[reg1_name] = registers[reg2_name]
        print(f"{reg1_name} = {registers[reg1_name]}")
        value_to_write = reg2_code
    else:  # op2 - число
        try:
            value = int(op2)
            registers[reg1_name] = value
            print(f"{reg1_name} = {value}")
            value_to_write = value
        except:
            print(f"Ошибка: неверный второй операнд {op2}")
            return 1
    
    # Синхронизация ВСЕХ регистров
    if reg1_code == 4:
        sync_ax_to_parts()
    elif reg1_code == 0 or reg1_code == 1:
        sync_parts_to_ax()
    elif reg1_code == 5:
        sync_bx_to_parts()
    elif reg1_code == 2 or reg1_code == 3:
        sync_parts_to_bx()
    
    # Запись в бинарный файл
    try:
        binaryd.write(struct.pack("<B", 0x20))
        binaryd.write(struct.pack("<B", flag))
        binaryd.write(struct.pack("<B", reg1_code))
        
        if flag == 1:  # 16-битная операция
            binaryd.write(struct.pack("<H", value_to_write))
        else:  # 8-битная операция
            binaryd.write(struct.pack("<B", value_to_write))
            
        return 0
    except Exception as e:
        print(f"ошибка записи: {e}")
        return 1
#===================================
def auto_save():
    global valid_save
    while not stop_thread.is_set():
        if stop_thread.wait(60):
            break
        temp_save = open("##temp##.bin", "wb+")
        try:
            binaryd.seek(0)
            save_out_file = binaryd.read()
            temp_save.write(save_out_file)
            binaryd.seek(0, 2)
            valid_save = True
        except:
            print("ошибка?")    
t = threading.Thread(target=auto_save)  
stop_thread = threading.Event()  
#===================================
print("> BOOT LETOS INTERPRETATOR v0.0.3\n===================\n|Start your work:)|\n===================")
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
    print("вы в файле:", file)  
    if new_project == False:
        temp_file = os.path.splitext(file)[0] + ".swg"
        comandfile = open(temp_file, "a") 
        load_variables_from_file(temp_file)
        try:
            PC_mem = max(addr for values in mem_for_variable.values() for _, addr in values)
        except:
            pass
    else:
        temp_file = os.path.splitext(file)[0] + ".swg"
        comandfile = open(temp_file, "w+")  
        new_project = False
    print("Ready for work!")    
    print("введите help для ознакомления")
    if "===================" in open(temp_file, "r").read():
        pass
    else:
        comandfile.write("===================")
        comandfile.write("\n")
    t.start()
#=================================== 
table_of_reg = {
    "AL": 0,
    "AH": 1,
    "BL": 2,
    "BH": 3,
    "AX": 4,
    "BX": 5,
    "al": 0,
    "ah": 1,
    "bl": 2,
    "bh": 3,
    "ax": 4,
    "bx": 5,
    "DS": 6,
    "ds": 6,
    "CS": 7,
    "cs": 7,
    
    # Обратное соответствие
    0: "AL",
    1: "AH", 
    2: "BL",
    3: "BH",
    4: "AX",
    5: "BX",
    6: "DS",
    7: "CS"
}

registers = {
    "AL": 0,
    "AH": 0,
    "BL": 0,
    "BH": 0,
    "AX": 0,
    "BX": 0,
    "DS": 4096,
    "CS": 0
}

bit16_registers = ("ax", "bx", "cs", "ds")
#===================================
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
#===================================
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
    
    number = struct.pack("<B", 0x09)
    binaryd.write(number)    
    binaryd.write(struct.pack("<B", reg_code))
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
    
    number = struct.pack("<B", 0x0A)
    binaryd.write(number)    
    binaryd.write(struct.pack("<B", reg_code))
    return

def add(reg1, reg2):
    global table_of_reg, registers
    reg1_code = valid_of_reg(reg1)
    reg2_code = valid_of_reg(reg2)
    
    if reg1_code == -1 or reg2_code == -1:
        print("Ошибка: неверный регистр")
        return
    
    reg1_name = table_of_reg[reg1_code].upper()
    
    if reg2_code != -1:  # reg2 - регистр
        reg2_name = table_of_reg[reg2_code].upper()
        registers[reg1_name] += registers[reg2_name]
    else:  # reg2 - число
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
    
    if reg2_code != -1:  # reg2 - регистр
        reg2_name = table_of_reg[reg2_code].upper()
        registers[reg1_name] -= registers[reg2_name]  
        print(f"{reg1_name} = {reg1_name} - {reg2_name} = {registers[reg1_name]}")
    else:  # reg2 - число
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
#===================================
def read():
    global valid_save
    binaryd.seek(0)
    index = 0
    stroke = binaryd.read()
    if len(stroke) == 0:
        print("файл пуст")
        return
    while True:

        os.system("cls" if os.name == "nt" else "clear")

        # выводим байты
        for b in stroke:
            print(f"0x{b:02x}", end=" ")
        print()

        size = len(stroke)

        # рисуем указатель
        for i in range(size):
            if i == index:
                print("^", end="    ")
            else:
                print(" ", end="    ")
        print()
        temp = ""
        key = keyboard.read_key()  
        if key == "left":
            index = (index - 1) % size
            time.sleep(0.15)
        elif key == "right":
            index = (index + 1) % size
            time.sleep(0.15)
        elif key in ("q", "esc"):
            os.system("cls" if os.name == "nt" else "clear")
            break
        elif key in "0123456789abcdef":
            while True:
                k = keyboard.read_key()
                if k == "enter":
                    save_out = False
                    break
                elif k in "0123456789abcdef":
                    if len(temp) >= 2:
                        pass
                    else:
                        time.sleep(0.15)
                        temp += k
                        print(k, end="", flush=True)
            try:
                if temp:
                    value = int(temp, 16)
                    binaryd.seek(index)
                    number = struct.pack("<B", value)
                    binaryd.write(number)
                    binaryd.flush()
                    return read()
            except:
                print("выход")
                break
    return          
#===================================    
def parse_number(s):
    if s is None:
        return 0
    try:
        # если есть префикс 0x → переводим как hex
        if s.startswith("0x") or s.startswith("0X"):
            return int(s, 16)
        else:
            return int(s)  # иначе decimal
    except ValueError:
        return s
#===================================
def switch(incode, operrand, operrand2):
    global file, comandfile, do_start, valid_save, mem
    global binaryd, PC_mem, mem_for_variable, temp_file
    binaryd.seek(0, 2)
    if operrand != 0:
        comandfile.write(str(incode)+" ")
        comandfile.write(str(operrand)+" ")
        comandfile.write(str(operrand2)+" ")
        comandfile.write('\n')
    if incode == "f":
        comandfile.write("stop")
        comandfile.write('\n')
    elif incode == "input":
        comandfile.write("input\n")    
    elif incode == "print":
        comandfile.write("print\n")    
    if incode == "open":
        file = input("напишите навзание файла для открытия(с .bin в конце для корректной работы):")
        try:
            binaryd = open(file, "r+b")
        except:
            file = "temp.bin"
            binaryd = open("temp.bin", "wb+")
        print("вы в файле:", file)  
        start()
    elif incode == "help":
        print("""               open — открывает бинарный файл для работы, если не существует — создаёт новый temp.bin.\n
            store — сохраняет значение регистра (или AL по умолчанию) в память по адресу.\n
            load — загружает значение из памяти в регистр.\n
            clear — очищает файлы программы (удаляет содержимое и пишет шапку).\n
            разослать — просто выводит шуточные тексты ("разосланы фотки стейси...").\n
            q — завершает программу, сохраняет все переменные и закрывает потоки.\n
            f / stop — записывает байт 0xFF (конец программы).\n
            cc — создаёт новый бинарный файл code.bin.\n
            mov — выполняет операцию перемещения данных между регистрами или значениями.\n
            si — сохраняет текущие бинарные данные в новый файл.\n
            so — сохраняет в наш файл код из другого.\n
            b — удаляет последний байт из бинарного файла.\n
            r — вызывает функцию read() (чтение).\n
            var — выводит все переменные из памяти (mem_for_variable).\n
            jmp — записывает команду безусловного перехода (jump).\n
            jz — записывает переход при нуле (jump if zero).\n
            jnz — записывает переход при ненуле (jump if not zero).\n
            add — складывает два регистра или значения.\n
            sub — вычитает один регистр (или значение) из другого.\n
            inc — увеличивает значение регистра на 1.\n
            dec — уменьшает значение регистра на 1.\n
            print — выводит значение регистра (вероятно AL).\n
            comp — сравнивает два регистра.\n
            PC — устанавливает указатель памяти PC_mem и сегмент данных DS.\n
            input — записывает команду ожидания пользовательского ввода.\n
            a = b - создание переменной.\n
            a *символ в переменной либо регистр* *регистр* - загружает определенный символ с переменной в регистр(счет символов начинается с 0)
            mem *число* - показывает что находиться в памяти по этому адресу
            reg - показывает регистры
            ---: - файл сохранен.\n
            ***: - файл не сохранен.\n""")
        return    
    elif incode == "store":
        valid_save = False
        try:
            a = valid_of_reg(operrand2)
            if a >= 0:
                binaryd.write(struct.pack("<B", 0x05))
                binaryd.write(struct.pack("<B", a))
                binaryd.write(struct.pack("<H", int(operrand)+registers["DS"]))
                mem[int(operrand)+registers["DS"]] = registers[table_of_reg[a]]
                PC_mem += 1
            else:
                binaryd.write(struct.pack("<B", 0x05))
                binaryd.write(struct.pack("<B", 0x00))
                binaryd.write(struct.pack("<H", int(operrand)+registers["DS"]))
                try: 
                    operrand2 = int(operrand2) 
                    mem[int(operrand)+registers["DS"]] = operrand2
                except:
                    print("введен неверный регистр")
                PC_mem += 1
        except:
            print("ошибка записи!")    
        return
    elif incode == "load":
        valid_save = False
        try:
            a = valid_of_reg(operrand)
            if a >= 0:
                binaryd.write(struct.pack("<B", 0x04))
                binaryd.write(struct.pack("<B", a))
                binaryd.write(struct.pack("<H", int(operrand2)+registers["DS"]))
                registers[table_of_reg[a]] = mem[int(operrand2)+registers["DS"]]
                PC_mem += 1
            else:
                print(a)
                print("ошибка регистра!")
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
            comandfile.close()
            comandfile = open(temp_file, "w+")
            comandfile.write("===================\n")
            print("удалено")
        else:
            print("отменено")
        return     
    elif incode == "разослать":
        try:
            if operrand == "школе":
                print("разосланы фотки стейси всей школе")
            elif operrand == "задире":
                print("респект от задиры получен")
            else:
                print("не разослано")   
        except:
            print("не разослано")    
        return
    elif incode == "mem":
        try:
            print(mem[int(operrand)+registers["DS"]])    
        except:
            print("указывайте правильную память")    
        return
    elif incode == "q":
        binaryd.seek(0, 2)
        stop_thread.set()
        print("Thank you for using Letos;)")
        for name, value in mem_for_variable.items():
            if name in open(temp_file, "r").read():
                pass
            else:
                comandfile.write("\n")
                # если обычная переменная: [value, addr]
                if isinstance(value[0], int):
                    val, addr = value
                    comandfile.write(f"{name}: value={val}, address={addr}")
                # если строка (список списков)
                else:
                    comandfile.write(f"{name}: ")
                    for val, addr in value:
                        comandfile.write(f"(value={val}, addr={addr}) ")   
        comandfile.write("\n")            
        do_start = False    
        t.join()
    elif incode == "f" or incode == "stop":
        valid_save = False
        number = struct.pack("<B", 0xFF)
        binaryd.write(number)
        return
    elif incode == "cc":
        try:
            code = open("code.bin", "wb+")
            print("файл создан")
            code.close
        except:
            print("неизвестная ошибка")
        return
    elif incode == "mov":
        valid_save = False
        mov(operrand, operrand2)
        return         
    elif incode == "si":
        valid_save = True
        if input("вы уверены?y/n:") == "y":
            try:
                binaryd.seek(0)
                save = input("в какой файл сохранить?(.bin в конце для корректной работы):")
                try:
                    binsave = open(save, "wb+")
                    binsave.write(binaryd.read())
                    binsave.close()
                    binaryd.seek(0, 2)
                    print("Успешно!")
                    
                except:
                    print("ошибка записи")    
            except:
                print("ошибка")
                
        else:
            print("изменения не сохранены")
        return   
    elif incode == "so":
        valid_save = True
        if input("вы уверены?y/n:") == "y":
            result = save_out()
            if result == 1:
                print("ошибка при записи файла, убедитесь что есть файл code.bin")
                
            elif result == 0:
                print("Успешно!")
                
            else:
                print("неизвестная ошибка?")
        else:
            print("изменения не сохранены")
        return  
    elif incode == "b":
        valid_save = False
        binaryd.seek(0, 2)
        size = binaryd.tell()
        binaryd.truncate(size -1)
        comandfile.write("X\n")
        return
    elif incode == "r":
        read()
        return
    elif incode == "var":
        print(mem_for_variable)    
        return
    elif incode == "jmp":
        valid_save = False
        number = struct.pack("<B", 0x0B)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        return
    elif incode == "jz":
        valid_save = False
        number = struct.pack("<B", 0x0C)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        return
    elif incode == "jnz":
        valid_save = False
        number = struct.pack("<B", 0x0D)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        return
    elif incode == "add":
        valid_save = False
        add(operrand, operrand2)
        if operrand in table_of_reg:
            number = struct.pack("<B", 0x08)
            binaryd.write(number)    
            temp1 = table_of_reg[operrand]
            number = struct.pack("<B", temp1)
            binaryd.write(number)  
        else:
            try:
                operrand = int(operrand)
            except:
                print("ошибка регистра")
                start()
            if operrand >= 0 or operrand <=7:
                number = struct.pack("<B", 0x08)
                binaryd.write(number)    
                temp1 = operrand
                number = struct.pack("<B", temp1)
                binaryd.write(number)  
            else:    
                print("ошибка первого операнда")
                start()
        if operrand2 in table_of_reg:
            temp1 = table_of_reg[operrand2]
            number = struct.pack("<B", temp1)
            binaryd.write(number)              
        else:
            try:
                operrand2 = int(operrand2)
            except:
                print("ошибка записи второго операнда")    
                start()
            temp1 = operrand2
            number = struct.pack("<B", temp1)
            binaryd.write(number)  

        return
    elif incode == "sub":
        valid_save = False
        sub(operrand, operrand2)
        if operrand in table_of_reg:
            number = struct.pack("<B", 0x07)
            binaryd.write(number)    
            temp1 = table_of_reg[operrand]
            number = struct.pack("<B", temp1)
            binaryd.write(number)  
        else:
            try:
                operrand = int(operrand)
            except:
                print("ошибка регистра")
                start()
            if operrand >= 0 or operrand <=7:
                number = struct.pack("<B", 0x07)
                binaryd.write(number)    
                temp1 = operrand
                number = struct.pack("<B", temp1)
                binaryd.write(number)  
            else:    
                print("ошибка первого операнда")
                start()
        if operrand2 in table_of_reg:
            temp1 = table_of_reg[operrand2]
            number = struct.pack("<B", temp1)
            binaryd.write(number)              
        else:
            try:
                operrand2 = int(operrand2)
            except:
                print("ошибка записи второго операнда")    
                start()
            temp1 = operrand2
            number = struct.pack("<B", temp1)
            binaryd.write(number)  
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
        number = struct.pack("<B", 0x0E)
        binaryd.write(number)    
        return           
    elif incode == "comp":
        valid_save = False
        number = struct.pack("<B", 0xD0)
        binaryd.write(number)    
        a = valid_of_reg(operrand)
        binaryd.write(struct.pack("<B", a))
        a = valid_of_reg(operrand2)
        binaryd.write(struct.pack("<B", a))
        return
    elif incode == "PC":
        switch("mov", "ds", int(operrand))
        PC_mem = int(operrand)    
        return
    elif incode == "input":   
        valid_save = False
        number = struct.pack("<B", 0xF9)
        binaryd.write(number)   
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
                        print("Failed!")
                        print("Ошибка загрузки символа в регистр!")   
                        return -1        
            else:
                try:
                    switch("mov", operrand2, mem_for_variable[incode][operrand][0])
                except:
                    print("Failed!")
                    print("Ошибка загрузки символа в регистр!")   
                    return -1        
        else:
            print("Failed!")
            print("неизвестный регистр, будет использоваться регистр al")    
            if isinstance(operrand, str):
                try:
                        switch("mov", operrand2, mem_for_variable[incode][registers[operrand.upper()]][0])
                except:
                        print("Failed!")
                        print("Ошибка загрузки символа в регистр!")   
                        return -1     
            else:
                try:
                    switch("mov", "al", mem_for_variable[incode][0][operrand])
                except:
                    print("Failed!")
                    print("Ошибка загрузки символа в регистр!")    
                    return -1        
        return 0            
    else:
        if operrand == "=":
            variable(incode, operrand2, operrand)     
        else:
            print("неизвсетная команда")
        return             
def variable(name, data, oper):
    global binaryd, comandfile, registers
    global PC_mem, mem_for_variable

    try:
        data = int(data)
    except:
        pass

    if oper == "=":
        if isinstance(data, str) or data > 255:
            # одиночный символ
            data = str(data)
            if len(data) == 1:
                value = ord(data)
                if name in mem_for_variable:
                    PC_mem = mem_for_variable[name][1]
                else:
                    PC_mem += 1

                mem_for_variable[name] = [value, PC_mem+registers["DS"]]
                switch("mov", "al", value)
                switch("store", PC_mem, value)
                return

            # строка
            else:
                if name not in mem_for_variable:
                    mem_for_variable[name] = []

                for ch in data:
                    value = ord(ch)
                    switch("mov", "al", value)
                    switch("store", PC_mem, value)
                    mem_for_variable[name].append([value, PC_mem-1+registers["DS"]])
                switch("sub", "al", "al")
                switch("store", PC_mem, "al")    
                mem_for_variable[name].append([0, PC_mem-1+registers["DS"]])
                return
                

        # число
        else:
            if name in mem_for_variable:
                PC_mem = mem_for_variable[name][1]
            else:
                PC_mem += 1

            mem_for_variable[name] = [int(data), PC_mem+registers["DS"]]
            switch("mov", "al", int(data))
            switch("store", PC_mem, int(data))
    return
#===================================        
def start():
    global valid_save
    if valid_save == True:
        ans = input("---:")
    else:
        ans = input("***:")
    parts = ans.split()
    command = parts[0] if parts else """"""
    opperand = 0
    opperand2 = 0
    if len(parts) > 1:
        opperand = parts[1]
    if len(parts) > 2:
        opperand2 = parts[2]    
    switch(command, opperand, opperand2)   
    while do_start:
        start()
#===================================
if do_start:
    start()    
