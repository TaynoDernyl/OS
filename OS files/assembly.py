import struct
import keyboard
import threading 
import os
import time
#===================================
def enter_for_read():
    try:
        temp = int(input(":"))
        return temp
    except:
        return 0    
#===================================
def auto_save():
    while not stop_thread.is_set():
        if stop_thread.wait(60):
            break
        temp_save = open("##temp##.bin", "wb+")
        try:
            binaryd.seek(0)
            save_out_file = binaryd.read()
            temp_save.write(save_out_file)
            binaryd.seek(0, 2)
        except:
            print("ошибка?")    
t = threading.Thread(target=auto_save)  
stop_thread = threading.Event()  
#===================================
file = input("напишите навзание файла для открытия:")
try:
    if file.endswith(".bin"):
        binaryd = open(file, "r+b")
    else:
        file = (file + ".bin")    
        binaryd = open(file, "r+b")
except:
    if (input("такого файла нет, хотите создать?y/n:")) == "y":
        if file.endswith(".bin"):
            binaryd = open(file, "wb+")
        else:
            file = (file + ".bin")    
            binaryd = open(file, "wb+")
    else:        
        file = "temp.bin"
        binaryd = open("temp.bin", "wb+")
        print("автоматически создан временный файл")
print("вы в файле:", file)  
t.start()
#=================================== 
registers = {
    "A": 0,
    "B": 0,
    "AL": 0,
    "AH": 0,
    "BL": 0,
    "BH": 0,
    "AX": 0,
    "BX": 0
}
#===================================
def save_out():
    try:
        temp = open(file, "wb+")
        save = input("откуда сохранить?(название через bin):")
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
def switch(incode, operrand):
    global file
    global binaryd
    binaryd.seek(0, 2)
    if incode == "sf":
        file = input("напишите навзание файла для открытия:")
        try:
            binaryd = open(file, "wb+")
        except:
            file = "temp.bin"
            binaryd = open("temp.bin", "wb+")
        print("вы в файле:", file)  
        start()
    elif incode == "clear":
        print("уверен?")
        temp = input("y/n:")
        if temp == "y" or temp == "Y" or temp == "yes":
            binaryd.truncate(0)
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
        number = struct.pack("<B", 0xFF)
        binaryd.write(number)
        stop_thread.set()
        t.join()
    elif incode == "comp":
        number = struct.pack("<B", 0xD0)
        binaryd.write(number)
        start()
    elif incode == "f":
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
    elif incode == "si":
        if input("вы уверены?y/n:") == "y":
            try:
                binaryd.seek(0)
                save = input("в какой файл сохранить?")
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
    elif incode == "":
        start()
    elif incode == "b":
        binaryd.seek(0, 2)
        size = binaryd.tell()
        binaryd.truncate(size -1)
        start() 
    elif incode == "r":
        read()
        start()
    elif incode == "jmp":
        number = struct.pack("<B", 0x0B)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()
    elif incode == "jz":
        number = struct.pack("<B", 0x0C)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()
    elif incode == "jnz":
        number = struct.pack("<B", 0x0D)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()
    elif incode == "lda":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0x01)
            binaryd.write(number)
            registers["A"] = operrand
            number = struct.pack("<B", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0x01)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                registers["A"] = operrand
                number = struct.pack("<B", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")    

        start()
    elif incode == "ldb":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0x02)
            binaryd.write(number)
            registers["B"] = operrand
            number = struct.pack("<B", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0x02)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                registers["B"] = operrand
                number = struct.pack("<B", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")   

        start()
    elif incode == "add":
        registers["A"] = registers["A"] + registers["B"]
        number = struct.pack("<B", 0x07)
        binaryd.write(number)    
        start()
    elif incode == "sub":
        registers["A"] = registers["A"] - registers["B"]
        number = struct.pack("<B", 0x08)
        binaryd.write(number)    
        start()    
    elif incode == "inc":
        registers["A"] += 1
        number = struct.pack("<B", 0x09)
        binaryd.write(number)    
        start()    
    elif incode == "dec":
        registers["A"] -= 1
        number = struct.pack("<B", 0x0A)
        binaryd.write(number)    
        start()    
    elif incode == "printl":
        number = struct.pack("<B", 0x0E)
        binaryd.write(number)    
        start()
    elif incode == "print":
        number = struct.pack("<B", 0xAE)
        binaryd.write(number)    
        start()   
    elif incode == "ldma":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0x03)
            binaryd.write(number)
            registers["A"] = operrand
            number = struct.pack("<H", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0x03)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                registers["A"] = operrand
                number = struct.pack("<H", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")
        start()
    elif incode == "stma":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0x05)
            binaryd.write(number)
            number = struct.pack("<H", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0x05)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                number = struct.pack("<H", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")
        start()
    elif incode == "ldmb":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0x04)
            binaryd.write(number)
            registers["B"] = operrand
            number = struct.pack("<H", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0x04)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                registers["B"] = operrand
                number = struct.pack("<H", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")
        start() 
    elif incode == "stmb":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0x06)
            binaryd.write(number)
            number = struct.pack("<H", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0x06)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                number = struct.pack("<H", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")
        start()    
    elif incode == "ldal":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0xA1)
            binaryd.write(number)
            registers["AL"] = operrand
            number = struct.pack("<B", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0xA1)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                registers["AL"] = operrand
                number = struct.pack("<B", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")
        start()
    elif incode == "ldah":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0xA2)
            binaryd.write(number)
            registers["AH"] = operrand
            number = struct.pack("<B", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0xA2)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                registers["AH"] = operrand
                number = struct.pack("<B", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")
        start()    
    elif incode == "ldbl":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0xB1)
            binaryd.write(number)
            registers["BL"] = operrand
            number = struct.pack("<B", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0xB1)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                registers["BL"] = operrand
                number = struct.pack("<B", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")
        start() 
    elif incode == "ldbh":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0xB2)
            binaryd.write(number)
            registers["BH"] = operrand
            number = struct.pack("<B", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0xB2)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                registers["BH"] = operrand
                number = struct.pack("<B", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")
        start()     
    elif incode == "ldax":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0xA3)
            binaryd.write(number)
            registers["AX"] = operrand
            number = struct.pack("<H", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0xA3)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                registers["AX"] = operrand
                number = struct.pack("<H", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")    
        start()        
    elif incode == "ldbx":
        if isinstance(operrand, int):
            number = struct.pack("<B", 0xB3)
            binaryd.write(number)
            registers["BX"] = operrand
            number = struct.pack("<H", operrand)
            binaryd.write(number)

        elif isinstance(operrand, str):
            if operrand.upper() in registers:
                number = struct.pack("<B", 0xB3)
                binaryd.write(number)
                operrand = registers[operrand.upper()]
                registers["BX"] = operrand
                number = struct.pack("<H", operrand)
                binaryd.write(number)
            else:
                print("неизвестный регистр")   
        start()                 
    else:
        start()        
#===================================        
def start():
    ans = input(":")
    parts = ans.split()
    command = parts[0]

    try:
        opperand = parse_number(parts[1])  
    except IndexError:
        opperand = 0

    switch(command, opperand)   
#===================================
def stop():
    pass        
#===================================
start()    