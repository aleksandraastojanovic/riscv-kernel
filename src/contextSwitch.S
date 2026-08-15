
#x1 je ra, x2 je sp cuva se od x3 do x31 nasops steku
.global pushRegisters
.type pushRegisters, @function
pushRegisters:
    addi sp,sp, -256
    .irp i, 3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\i, \i * 8(sp)
    .endr
    ret

.global popRegisters
.type popRegisters, @function
popRegisters:
    .irp i, 3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\i, \i * 8(sp)
    .endr
    addi sp,sp,256
    ret

#kontekst switch stari na a0 novi a1
.global contextSwitch
.type contextSwitch,@function
contextSwitch:
    sd ra,0 * 8(a0) #pamti gde je stala stara nit
    sd sp, 1 * 8(a0) #ipamti sp
    ld ra, 0* 8(a1) #nastavlja novi gde je satala
    ld sp, 1 * 8(a1)
    ret