#include <p33FJ32MC302.h>

_FOSCSEL(FNOSC_FRCPLL);
_FOSC(FCKSM_CSDCMD & OSCIOFNC_ON & POSCMD_NONE);
_FWDT(FWDTEN_OFF);
_FGS(GCP_OFF);

void setup_clock() {
    CLKDIVbits.PLLPRE = 0;
    CLKDIVbits.PLLPOST = 0;
    PLLFBD = 43;
}

void delay_ms(unsigned long ms) {
    uint64_t nops = (uint64_t) (39613750ULL) * (uint64_t) ms / 1000ULL;
    __delay32(nops);
}

#define LED12 (LATBbits.LATB12)
#define LED13 (LATBbits.LATB13)
#define LED14 (LATBbits.LATB14)
#define LED15 (LATBbits.LATB15)

void write_led(int v) {
    LED15 = (v >> 3) & 1;
    LED14 = (v >> 2) & 1;
    LED13 = (v >> 1) & 1;
    LED12 = (v >> 0) & 1;
}

#define BUTTON0 (PORTBbits.RB0) 
#define BUTTON1 (PORTBbits.RB1) 

void setup(void) {
    setup_clock();

    /* setup pins */
    TRISA = 0x0003; // 1: input, 0: output
    TRISB = 0x0103;
    AD1PCFGL = 0xfffc; // use RB1, RB0 as analong inputs
}

#define PRAC_N 5

void loop(void) {
#if PRAC_N == 1
    if (BUTTON0) {
        write_led(0xf);
        delay_ms(1000);
        write_led(0x0);
        delay_ms(1000);
    } else if (BUTTON1) {
        write_led(0xf);
        delay_ms(2000);
        write_led(0x0);
        delay_ms(2000);
    }
#elif PRAC_N == 2
    for (int i = 0; i < 4; i++) {
        LED15 = (i & 1) && BUTTON0;
        LED14 = (i & 2) && BUTTON1;
        delay_ms(1000);
    }
#elif PRAC_N == 3
    // ?
    if (BUTTON0) {
        for (int i = 0; i <= 4; i++) {
            write_led(1 << i);
            delay_ms(500);
        }
    } else if (BUTTON1) {
        for (int i = 0; i <= 4; i++) {
            write_led(1 << (3 - i));
            delay_ms(500);
        }
    }
#elif PRAC_N == 4
    if (BUTTON0) {
        LED15 = 1;
        for (;;);
    }
    if (BUTTON1) {
        LED14 = 1;
        for (;;);
    }
#elif PRAC_N == 5
    if (BUTTON0 || BUTTON1) {
        LED15 = 1;
        for(;;);
    }
#endif
}

int main(void) {
    setup();
    for (;;) {
        loop();
    }
}