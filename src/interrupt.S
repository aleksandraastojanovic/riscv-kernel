.extern handleSupervisorTrap
.align 4
.global supervisorTrap
.type supervisorTrap, @function
supervisorTrap:
    addi sp,sp,-256 //32 registra mesto
    .irp i, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\i, \i * 8(sp)
    .endr

    mv a0, sp //saljese adresa niza c++ funkciji
    call handleSupervisorTrap

    .irp i, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\i, \i * 8(sp)
    .endr
    addi sp,sp,256
    sret