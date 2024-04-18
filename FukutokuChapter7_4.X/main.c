#include <p33FJ32MC302.h>
#include <stdio.h>

/* 
 * Global Configuration 
 */

_FOSCSEL(FNOSC_FRCPLL);
_FOSC(FCKSM_CSDCMD & OSCIOFNC_ON & POSCMD_NONE);
_FWDT(FWDTEN_OFF);
_FGS(GCP_OFF);

/* 
 * Clock Configuration 
 */

#define CLK_PLL_SCALE 33

void setup_clock(void) {
    CLKDIVbits.PLLPRE = 0; // PLLPRE 2
    CLKDIVbits.PLLPOST = 0; // PLLPOST 2
    PLLFBD = CLK_PLL_SCALE - 2;
    RCONbits.SWDTEN = 0;
}

#define CLK_FREQ ((uint64_t)((7370000ULL * (uint64_t) CLK_PLL_SCALE) >> 2))
#define INST_FREQ ((uint64_t)(CLK_FREQ >> 1))

/* 
 * UART Configuration 
 */

#define SERIAL_BAUDRATE 9600

void setup_serial(void) {
    U1MODE = 0x8808;
    U1STA = 0x0400;
    U1BRG = (INST_FREQ / 4 / SERIAL_BAUDRATE) - 1;
}

/* 
 * Serail
 */

void serial_put_byte(uint8_t value) {
    while (U1STAbits.UTXBF);
    U1TXREG = value;
}

uint8_t serial_get_byte(void) {
    uint16_t value;
    for (;;) {
        while (!U1STAbits.URXDA);
        unsigned char fail = 0;
        if (U1STAbits.OERR) {
            U1STAbits.OERR = 0;
            fail = 1;
        }
        if (U1STAbits.FERR) {
            value = U1RXREG;
            U1STAbits.FERR = 0;
            fail = 1;
        }
        if (fail) {
            continue;
        }
        value = U1RXREG;
        return value & 0xff;
    }
}

/* 
 * Printf Handler
 */

void putch(unsigned char ch) {
    serial_put_byte((uint8_t) ch);
}

/* 
 * Delay
 */

void delay_ms(unsigned int ms) {
    static const uint32_t n_inst = INST_FREQ / 1000UL;
    while (ms--) {
        __delay32(n_inst);
    }
}

void delay_us(unsigned int us) {
    static const uint32_t n_inst = INST_FREQ / 1000UL / 1000UL;
    while (us--) {
        __delay32(n_inst);
    }
}

/* 
 * Setup
 */
void setup(void) {
    setup_clock();
    setup_serial();

    /* setup I/Os */
    _U1RXR = 9; // UART input
    _RP8R = 3; // UART output
    TRISA = 0x001f; // 1: input, 0: output
    TRISB = 0x0203;

    T1CONbits.TCS = 0; // internal
    T1CONbits.TCKPS = 0; // x1
    PR1 = INST_FREQ * 50UL / 1000UL / 1000UL;
    _T1IE = 1;
    _T1IF = 0;
    T1CONbits.TON = 1;
}

/* 
 * (User)
 */
void _ISR _T1Interrupt(void) {
    _T1IF = 0;
    _LATB15 = ~_LATB15;
}

void main(void) {
    setup();

    for (;;) {
    }
}