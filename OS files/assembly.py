import struct
import keyboard

binaryd = open("temp.bin", "wb+")
#===================================
def save_out():
    try:
        temp = open("temp.bin", "wb+")
        code = open("code.bin", "rb")
        data = code.read()
        temp.write(data)
        code.close()
        return 0
    except:
        return 1    
#===================================
def read():
    temp = keyboard.read_key()
    binaryd.seek(0)
    stroke = binaryd.read()
    for b in stroke:
        print(hex(b), end=" ")  
    print()
    return 0
#===================================    
def parse_number(s):
    # если есть префикс 0x → переводим как hex
    if s.startswith("0x") or s.startswith("0X"):
        return int(s, 16)
    return int(s)  # иначе decimal

#===================================
def switch(incode, operrand):
    if incode == "q":
        binaryd.seek(0, 2)
        number = struct.pack("<B", 0xFF)
        binaryd.write(number)
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
                binsave = open("code.bin", "wb+")
                binsave.write(binaryd.read())
                binsave.close()
                binaryd.seek(0, 2)
                print("Успешно!")
                start()
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
        size = binaryd.tell()
        binaryd.truncate(size -1)
        binaryd.seek(0, 2)
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
        number = struct.pack("<B", 0x01)
        binaryd.write(number)
        number = struct.pack("<B", operrand)
        binaryd.write(number)
        start()
    elif incode == "ldb":
        number = struct.pack("<B", 0x02)
        binaryd.write(number)
        number = struct.pack("<B", operrand)
        binaryd.write(number)
        start()
    elif incode == "add":
        number = struct.pack("<B", 0x07)
        binaryd.write(number)    
        start()
    elif incode == "sub":
        number = struct.pack("<B", 0x08)
        binaryd.write(number)    
        start()    
    elif incode == "inc":
        number = struct.pack("<B", 0x09)
        binaryd.write(number)    
        start()    
    elif incode == "dec":
        number = struct.pack("<B", 0x0A)
        binaryd.write(number)    
        start()    
    elif incode == "print":
        number = struct.pack("<B", 0x0E)
        binaryd.write(number)    
        start()
    elif incode == "printl":
        number = struct.pack("<B", 0xAE)
        binaryd.write(number)    
        start()   
    elif incode == "ldma":
        number = struct.pack("<B", 0x03)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()
    elif incode == "stma":
        number = struct.pack("<B", 0x05)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()
    elif incode == "ldmb":
        number = struct.pack("<B", 0x04)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()    
    elif incode == "stmb":
        number = struct.pack("<B", 0x06)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()  
    elif incode == "ldal":
        number = struct.pack("<B", 0xA1)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()
    elif incode == "ldah":
        number = struct.pack("<B", 0xA2)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start()    
    elif incode == "ldbl":
        number = struct.pack("<B", 0xB1)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
        start() 
    elif incode == "ldbh":
        number = struct.pack("<B", 0xB2)
        binaryd.write(number)
        number = struct.pack("<H", operrand)
        binaryd.write(number)
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
start()    