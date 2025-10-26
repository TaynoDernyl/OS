passwd = hello

; Проверка 1-го символа
load ah 0    ; ожидаемый 'h'
input
comp al ah
jnz end      ; если НЕ равны -> завершить
             ; если равны -> продолжить

; Проверка 2-го символа  
step2:
load ah 1    ; ожидаемый 'e'
input
comp al ah
jnz end      ; если НЕ равны -> завершить

; Проверка 3-го символа
step3:
load ah 2    ; ожидаемый 'l'
input
comp al ah
jnz end      ; если НЕ равны -> завершить

; Проверка 4-го символа
step4:
load ah 3    ; ожидаемый 'l'
input
comp al ah
jnz end      ; если НЕ равны -> завершить

; Проверка 5-го символа
step5:
load ah 4    ; ожидаемый 'o'
input
comp al ah
jnz end      ; если НЕ равны -> завершить

; Все символы верны!
yay:
mov al 13
print
mov al 10
print
mov bh passwd
printstr
mov al 33
print
jmp accept

end:
stop

accept:
stop