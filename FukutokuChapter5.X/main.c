#include <p33FJ32MC302.h>
#include <stdio.h>

_FOSCSEL(FNOSC_FRCPLL);
_FOSC(FCKSM_CSDCMD & OSCIOFNC_ON & POSCMD_NONE);
_FWDT(FWDTEN_OFF);
_FGS(GCP_OFF);

#define CLK_PLL_SCALE 33

void setup_clock(void) {
    CLKDIVbits.PLLPRE = 0; // PLLPRE 2
    CLKDIVbits.PLLPOST = 0; // PLLPOST 2
    PLLFBD = CLK_PLL_SCALE - 2;
}

#define CLK_FREQ ((uint64_t)((7370000ULL * (uint64_t) CLK_PLL_SCALE) >> 2))
#define INST_FREQ ((uint64_t)(CLK_FREQ >> 1))

#define SERIAL_BAUDRATE 9600

void setup_serial(void) {
    U2MODE = 0x8808;
    U2STA = 0x0400;
    U2BRG = (INST_FREQ / 4 / SERIAL_BAUDRATE) - 1;
}

void delay_ms(const unsigned int ms) {
    static const uint64_t u64_inst_freq = INST_FREQ;
    const uint32_t inst = u64_inst_freq * (uint64_t) ms / 1000ULL;
    __delay32(inst);
}

void setup(void) {
    setup_clock();
    setup_serial();

    /* setup I/Os */
    _U2RXR = 9; // UART input
    _RP8R = 5; // UART output
    TRISA = 0x001f; // 1: input, 0: output
    TRISB = 0x0203;
    AD1PCFGL = 0xfffc; // use RB1, RB0 as analong inputs
}

void serial_put(const char ch) {
    while (U1STAbits.UTXBF);
    U2TXREG = ch;
}

void serial_write(const char *str) {
    for (char *p = str; *p; p++) {
        serial_put(*p);
    }
}

char serial_get(void) {
    uint16_t data;
    for (;;) {
        while (!U2STAbits.URXDA);
        unsigned char fail = 0;
        if (U2STAbits.OERR) {
            U2STAbits.OERR = 0;
            fail = 1;
        }
        if (U2STAbits.FERR) {
            data = U2RXREG;
            U2STAbits.FERR = 0;
            fail = 1;
        }
        if (fail) {
            continue;
        }
        data = U2RXREG;
        return data;
    }
}

int main() {
    setup();

    LATB = 10 << 12;
    delay_ms(100);
    LATB = 0;
    delay_ms(100);

    LATB = 10 << 12;
    delay_ms(100);
    LATB = 0;
    delay_ms(100);


    for (;;) {
        char ch = serial_get();
        LATB = (unsigned char) ch << 12;
        serial_put(ch);
    }
    return 1;
}
