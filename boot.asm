	bits 16
	org 0x7c00
	jmp start

msgH:
	db "Hello, this is IvOS", 0x0A, 0x0D, "Enter the password:", 0x00 ; приветственное сообщение

done:
	jmp $

putchar:
	mov ah, 0x0E
	mov bh, 0
	int 10h
	ret

puts:
	.loop:
	lodsb
	or al, al
	jz .done
	call putchar
	jmp .loop
	.done:
	jmp input

start:
	mov si, buffer
	xor ax, ax
	mov ds, ax
	mov dx, 2000
	.loop:
	mov al, 0x20
	call putchar

	test dx, dx
	dec dx
	jnz .loop
	jmp .done

	.done:
	mov si, msgH
	call puts

buffer:
	times 10 db 0
input:
	mov ah, 0
	int 0x16
	mov [si], al
	cmp al, 0x0D
	je .done
	inc si
	mov ah, 0x0e
	int 0x10
	jmp input

	.done:
	mov ah, 0x0e
	int 0x10
	jmp start

times 510-($-$$) db 0x00
dw 0AA55h
