mov bl 80 
mov cx 6

downline: 
setv 9 11 39
inc px
comp px bl
jnz downline

upline:
setv 9 0 39
inc px
comp px bl
jnz upline

render

left:
sub al al
mov py cx
setv 60 10 0
setv 20 10 30
render
input
comp al 97
jz left
comp al 65
jz left
comp al 100
jz right
comp al 68
jz right
comp al 113
jz stop
comp al 81
jz stop
comp al 13
mov al 35
jz enter
jnz none

right:
sub al al
mov py cx
setv 20 10 0
setv 60 10 30
render
input
comp al 97
jz left
comp al 65
jz left
comp al 100
jz right
comp al 68
jz right
comp al 113
jz stop
comp al 81
jz stop
comp al 13
mov al 33
jz enter
jnz none

none:
sub al al
mov py cx
setv 20 10 0
setv 60 10 0
render
input
comp al 97
jz left
comp al 65
jz left
comp al 100
jz right
comp al 68
jz right
comp al 113
jz stop
comp al 81
jz stop
comp al 13
mov al 42
jz enter
jnz none

stop:
stop

enter:
print
jmp stop