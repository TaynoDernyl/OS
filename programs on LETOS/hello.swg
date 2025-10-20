===================
text = hello 
mov al 104 
mov al 101 
store 1 101 
mov al 108 
store 2 108 
mov al 108 
store 3 108 
mov al 111 
store 4 111 
sub al al 
store 5 al 
text 0 0 
print
text 1 0 
print
text 2 0 
print
text 3 0 
print
text 4 0 
print
stop

text: (value=104, addr=0) (value=101, addr=1) (value=108, addr=2) (value=108, addr=3) (value=111, addr=4) (value=0, addr=6) 