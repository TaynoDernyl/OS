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
PC_mem = 4096
valid_save = True
#===================================
do_start = True
new_project = False
#===================================
def valid_of_reg(reg):
    if reg in table_of_reg:
        return table_of_reg[reg]
    elif reg >= 0 and reg < 8:
        return reg
    else:
        return -1
#===================================
def mov(op1, op2):
    if op1 in bit16_registers:
        flag = 1
    else:
        flag = 0
    if op1 in table_of_reg:
            op1 = table_of_reg[op1]
            if op2 in table_of_reg:
                op2 = table_of_reg[op2]
            else:
                op2 = int(op2)
            if flag == 0 :    
                try:
                    binaryd.write(struct.pack("<B", 0x20))
                    binaryd.write(struct.pack("<B", flag))
                    binaryd.write(struct.pack("<B", op1))
                    binaryd.write(struct.pack("<B", op2))
                    return 0
                except:
                    print("ошибка 1")
                    return 1
            if flag == 1 :    
                try:
                    print(op1)
                    if op1 == 6:
                        print(op2)
                        if not op2 >= 0 and op2 <= 7:
                            registers["DS"] = op2
                            return 0
                        else:
                            print("Failed!")
                            print("Загрузка сегментных регистров недоступна из других регистров")    
                            return -1
                    if op1 == 7:
                        print(op2)
                        if not op2 >= 0 and op2 <= 7:
                            registers["CS"] = op2
                            return 0
                        else:
                            print("Failed!")
                            print("Загрузка сегментных регистров недоступна из других регистров")    
                            return -1    
                    binaryd.write(struct.pack("<B", 0x20))
                    binaryd.write(struct.pack("<B", flag))
                    binaryd.write(struct.pack("<B", op1))
                    binaryd.write(struct.pack("<H", op2))
                    return 0
                except:
                    print("ошибка 1")
                    return 1    
    else:
            op1 = int(op1)
            if op1 >= 0 and op1 <=7:
                if op2 in table_of_reg:
                    op2 = table_of_reg[op2]
                else:
                    op2 = int(op2)   
                if flag == 0: 
                    try:
                        binaryd.write(struct.pack("<B", 0x20))
                        binaryd.write(struct.pack("<B", flag))
                        binaryd.write(struct.pack("<B", op1))
                        binaryd.write(struct.pack("<B", op2))
                        return 0
                    except:
                        print("ошибка 2")
                        return 1
                if flag == 1: 
                    try:
                        if op1 == 6:
                            if valid_of_reg(op2) != -1:
                                registers["DS"] = op2
                            else:
                                print("Failed!")
                                print("Загрузка сегментных регистров недоступна из других регистров")    
                                return -1
                        binaryd.write(struct.pack("<B", 0x20))
                        binaryd.write(struct.pack("<B", flag))
                        binaryd.write(struct.pack("<B", op1))
                        binaryd.write(struct.pack("<H", op2))
                        return 0
                    except:
                        print("ошибка 2")
                        return 1    
            else:    
                print("введен неверный регистр!")
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
    else:
        temp_file = os.path.splitext(file)[0] + ".swg"
        comandfile = open(temp_file, "w+")  
        new_project = False
    print("Ready for work!")    
    print("введите help для ознакомления")
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
    "cs": 7
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
def read():
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
    global file, comandfile, do_start, valid_save
    global binaryd, PC_mem, mem_for_variable
    
    if operrand and operrand2 != 0:
        comandfile.write(str(incode)+" ")
        comandfile.write(str(operrand)+" ")
        comandfile.write(str(operrand2)+" ")
        comandfile.write('\n')
    if incode == "f":
        comandfile.write("stop")
        comandfile.write('\n')
    binaryd.seek(0, 2)
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
        print("""open — открывает бинарный файл для работы, если не существует — создаёт новый temp.bin.\n
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
                PC_mem += 1
            else:
                print("будет записан регистр al")
                binaryd.write(struct.pack("<B", 0x05))
                binaryd.write(struct.pack("<B", 0x00))
                binaryd.write(struct.pack("<H", int(operrand)+registers["DS"]))
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
                PC_mem += 1
            else:
                print(a)
                print("ошибка регистра!")
        except:
            print("ошибка записи!")    
        return
    elif incode == "clear":
        valid_save = False
        print("уверен?")
        temp = input("y/n:")
        if temp == "y" or temp == "Y" or temp == "yes":
            PC_mem = registers["DS"]
            binaryd.truncate(0)
            comandfile.close()
            comandfile = open(temp_file, "w+")
            comandfile.write("===================\n")
            print("удалено")
        else:
            print("отменено")
        start()      
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
        start()
    elif incode == "q":
        binaryd.seek(0, 2)
        stop_thread.set()
        print("Thank you for using Letos;)")
        for name, value in mem_for_variable.items():
            comandfile.write("\n")
            # если обычная переменная: [value, addr]
            if isinstance(value[0], int):
                val, addr = value
                comandfile.write(f"{name}: value={val}, address={addr-1}")
            # если строка (список списков)
            else:
                comandfile.write(f"{name}: ")
                for val, addr in value:
                    comandfile.write(f"(value={val}, addr={addr-1}) ")
        comandfile.write("\n")            
        comandfile.write("===================")            
        do_start = False    
        t.join()
    elif incode == "f" or incode == "stop":
        valid_save = False
        number = struct.pack("<B", 0xFF)
        binaryd.write(number)
        start()
    elif incode == "cc":
        try:
            code = open("code.bin", "wb+")
            print("файл создан")
            code.close
            start()
        except:
            print("неизвестная ошибка")
            start()   
    elif incode == "mov":
        valid_save = False
        try:
            prost = mov(operrand, operrand2)
            if prost == -1:
                comandfile.write("Failed operation!")
                comandfile.write("\n")
        except:
            print("ошибка")
        return         
    elif incode == "si":
        valid_save = True
        if input("вы уверены?y/n:") == "y":
            try:
                binaryd.seek(0)
                save = input("в какой файл сохранить?(.bin в конце для корректной работы)")
                try:
                    binsave = open(save, "wb+")
                    binsave.write(binaryd.read())
                    binsave.close()
                    binaryd.seek(0, 2)
                    print("Успешно!")
                    start()
                except:
                    print("ошибка записи")    
            except:
                print("ошибка")
                start()
        else:
            print("изменения не сохранены")
            start()    
    elif incode == "so":
        valid_save = True
        if input("вы уверены?y/n:") == "y":
            result = save_out()
            if result == 1:
                print("ошибка при записи файла, убедитесь что есть файл code.bin")
                start()
            elif result == 0:
                print("Успешно!")
                start()
            else:
                print("неизвестная ошибка?")
        else:
            print("изменения не сохранены")
            start()   
    elif incode == "b":
        valid_save = False
        binaryd.seek(0, 2)
        size = binaryd.tell()
        binaryd.truncate(size -1)
        start() 
    elif incode == "r":
        read()
        start()
    elif incode == "var":
        print(mem_for_variable)    
        start()
    elif incode == "jmp":
        valid_save = False
        number = struct.pack("<B", 0x0B)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()
    elif incode == "jz":
        valid_save = False
        number = struct.pack("<B", 0x0C)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()
    elif incode == "jnz":
        valid_save = False
        number = struct.pack("<B", 0x0D)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()
    elif incode == "add":
        valid_save = False
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

        start()
    elif incode == "sub":
        valid_save = False
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
        number = struct.pack("<B", 0x09)
        binaryd.write(number)    
        a = valid_of_reg(operrand)
        binaryd.write(struct.pack("<B", a))
        start()    
    elif incode == "dec":
        valid_save = False
        number = struct.pack("<B", 0x0A)
        binaryd.write(number)    
        a = valid_of_reg(operrand)
        binaryd.write(struct.pack("<B", a))
        start()    
    elif incode == "print":
        valid_save = False
        number = struct.pack("<B", 0x0E)
        binaryd.write(number)    
        start()            
    elif incode == "comp":
        valid_save = False
        number = struct.pack("<B", 0xD0)
        binaryd.write(number)    
        a = valid_of_reg(operrand)
        binaryd.write(struct.pack("<B", a))
        a = valid_of_reg(operrand2)
        binaryd.write(struct.pack("<B", a))
        start()
    elif incode == "PC":
        switch("mov", "ds", int(operrand))
        PC_mem = int(operrand)    
        start()
    elif incode == "input":
        valid_save = False
        number = struct.pack("<B", 0xF9)
        binaryd.write(number)   
        start()
    else:
        if operrand == "=":
            variable(incode, operrand2, operrand)     
        else:
            pass
        start()               
def variable(name, data, oper):
    global binaryd, comandfile
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

                mem_for_variable[name] = [value, PC_mem]
                switch("mov", "al", value)
                switch("store", PC_mem, value)
                PC_mem += 1
                return

            # строка
            else:
                if name not in mem_for_variable:
                    mem_for_variable[name] = []

                for ch in data:
                    value = ord(ch)
                    switch("mov", "al", value)
                    switch("store", PC_mem, value)
                    mem_for_variable[name].append([value, PC_mem])
                    PC_mem += 1
                    print(PC_mem)
                switch("sub", "al", "al")
                switch("store", PC_mem, "al")    
                mem_for_variable[name].append([0, PC_mem])
                PC_mem += 1
                return
                

        # число
        else:
            if name in mem_for_variable:
                PC_mem = mem_for_variable[name][1]
            else:
                PC_mem += 1

            mem_for_variable[name] = [int(data), PC_mem]
            switch("mov", "al", int(data))
            switch("store", PC_mem, int(data))
            PC_mem += 1
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
