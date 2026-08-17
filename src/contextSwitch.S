
.global contextSwitch
.type contextSwitch, @function
contextSwitch:
    sd ra, 0 * 8(a0)
    sd sp, 1 * 8(a0)
    .irp i, 0,1,2,3,4,5,6,7,8,9,10,11
    sd s\i, (\i + 2) * 8(a0)
    .endr

    ld ra, 0 * 8(a1)
    ld sp, 1 * 8(a1)
    .irp i, 0,1,2,3,4,5,6,7,8,9,10,11
    ld s\i, (\i + 2) * 8(a1)
    .endr

    ret                   # povratak u novu nit

