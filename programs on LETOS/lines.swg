mov bl 80
downline:
setv 9 11 100
inc px
comp px bl
jnz downline
upline:
setv 9 0 100
inc px
comp px bl
jnz upline
end:
render
stop