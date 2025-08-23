import struct
binaryd = open("code.bin", "wb+")
#===================================
def switch(incode, operrand):
    if incode == "q":
        number = struct.pack("<B", 0xFF)
        binaryd.write(number)
    elif incode == "":
        start()
    elif incode == "b":
        binaryd.seek(0, 2)
        size = binaryd.tell()
        binaryd.truncate(size -1)
        start() 
    elif incode == "r":
        binaryd.seek(0)
        stroke = binaryd.read()
        print(stroke)
        binaryd.seek(0, 2)
        stroke = binaryd.read()
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
    else:
        start()        
#===================================        
def start():
    ans = input(":")
    parts = ans.split()
    command = parts[0]
    try:
        opperand = int(parts[1])
        switch(command, opperand)
    except:
        switch(command, 0)
#===================================        
start()    