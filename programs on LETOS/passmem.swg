passwd = hello ; Наш пароль
mov bl 8
step1:
input ; ввод
comp al bl
jz step1 ; если backspace то заново
print
store 7 al ; загрузка в память ввод
step2:
input ; ввод
comp al bl
jz step2 ; если backspace то заново
print
store 8 al ; загрузка в память ввод
step3:
input ; ввод
comp al bl
jz step3 ; если backspace то заново
print
store 9 al ; загрузка в память ввод
step4:
input ; ввод
comp al bl
jz step4 ; если backspace то заново
print
store 10 al ; загрузка в память ввод
step5:
input ; ввод
comp al bl
jz step5 ; если backspace то заново
print
store 11 al ; загрузка в память ввод

validpass: ; сверяем наши пароли
load al 7
load ah 0
comp al ah
jnz end
load al 8
load ah 1
comp al ah
jnz end
load al 9
load ah 2
comp al ah
jnz end
load al 10
load ah 3
comp al ah
jnz end
load al 11
load ah 4
comp al ah
jnz end

yay:
pops = true! ;вывод если пароли совпадают
mov al 13
print
mov al 10
print
mov bh pops
printstr
f

end:
noch = false! ;вывод если пароли не совпадают
mov al 13
print
mov al 10
print
mov bh noch
printstr
f