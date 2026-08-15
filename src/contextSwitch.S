
#kontekst switch stari na a0 novi a1
.global contextSwitch
.type contextSwitch,@function
contextSwitch:
    sd ra,0 * 8(a0) #pamti gde je stala stara nit
    sd sp, 1 * 8(a0) #ipamti sp
    ld ra, 0* 8(a1) #nastavlja novi gde je satala
    ld sp, 1 * 8(a1)
    ret