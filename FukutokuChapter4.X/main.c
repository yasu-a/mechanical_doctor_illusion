#include <p33FJ32MC302.h>

_FOSCSEL(FNOSC_FRCPLL);
_FOSC(FCKSM_CSDCMD & OSCIOFNC_ON & POSCMD_NONE);
_FWDT(FWDTEN_OFF);
_FGS(GCP_OFF);

#define CLK_PLL_SCALE 17

void setup_clock(void) {
    CLKDIVbits.PLLPRE = 0; // PLLPRE 2
    CLKDIVbits.PLLPOST = 0; // PLLPOST 2
    PLLFBD = CLK_PLL_SCALE - 2;
}

#define CLK_FREQ ((uint64_t)((7370000ULL * (uint64_t) CLK_PLL_SCALE) >> 2))
#define INST_FREQ ((uint64_t)(CLK_FREQ >> 1))

void delay_ms(const unsigned int ms) {
    static const uint64_t u64_inst_freq = INST_FREQ;
    const uint32_t inst = u64_inst_freq * (uint64_t) ms / 1000ULL;
    __delay32(inst);
}

#define LED12 (LATBbits.LATB12)
#define LED13 (LATBbits.LATB13)
#define LED14 (LATBbits.LATB14)
#define LED15 (LATBbits.LATB15)

void write_led(uint16_t v) {
    LED15 = (v >> 3) & 1;
    LED14 = (v >> 2) & 1;
    LED13 = (v >> 1) & 1;
    LED12 = (v >> 0) & 1;
}

#define BUZZER (LATBbits.LATB7)

#define BUTTON0 (PORTBbits.RB0) 
#define BUTTON1 (PORTBbits.RB1) 

void setup(void) {
    setup_clock();

    /* setup I/Os */
    TRISA = 0x0003; // 1: input, 0: output
    TRISB = 0x0103;
    AD1PCFGL = 0xfffc; // use RB1, RB0 as analong inputs
}

#define PRACTICE_NUMBER 4

#if PRACTICE_NUMBER == 1

void _ISR _T4Interrupt(void) {
    LED12 = ~LED12;
    _T4IF = 0;
}

void main(void) {
    setup();

    /* setup timer-4 */
    T4CONbits.TON = 1;
    T4CONbits.TCS = 0; // internal
    T4CONbits.TCKPS = 2; // x64
    PR4 = INST_FREQ * 250UL / 1000UL / 64UL; // counts per 0.25s
    _T4IE = 1;
    _T4IF = 0;

    /* loop */
    for (;;);
}

#elif PRACTICE_NUMBER == 2

void _ISR _T4Interrupt(void) {
    LED12 = ~LED12;
    _T4IF = 0;
}

void _ISR _T3Interrupt(void) {
    LED15 = ~LED15;
    _T3IF = 0;
}

void main(void) {
    setup();

    /* setup timer-2/3 as a 32bit timer */
    T2CONbits.TCS = 0; // internal
    T2CONbits.TCKPS = 2; // x64
    T2CONbits.T32 = 1;
    uint32_t u32_pr = INST_FREQ * 2UL / 64UL; // counts per 2s
    PR2 = (uint16_t) u32_pr;
    PR3 = (uint16_t) (u32_pr >> 16);
    _T3IF = 0;
    _T3IE = 1;
    T2CONbits.TON = 1;

    /* setup timer-4 */
    T4CONbits.TCS = 0; // internal
    T4CONbits.TCKPS = 3; // x256
    PR4 = INST_FREQ / 256UL * 500UL / 1000UL; // counts per 500ms
    _T4IF = 0;
    _T4IE = 1;
    T4CONbits.TON = 1;

    /* loop */
    for (;;);
}

#elif PRACTICE_NUMBER == 3

volatile unsigned short state = 0;

void _ISR _INT1Interrupt(void) {
    state += 1;
    _INT1IF = 0;
}

void main(void) {
    setup();

    /* setup external interruption */
    _INT1R = 0; // RB0 as source
    _INT1EP = 0; // detect rising edge
    _INT1IF = 0;
    _INT1IE = 1;

    /* loop */
    for (;;) {
        write_led(state & 0xf);
    }
}

#elif PRACTICE_NUMBER == 4

void _ISR _T2Interrupt(void) {
    LED12 = ~LED12;
    _T2IF = 0;
}

void _ISR _T3Interrupt(void) {
    LED15 = ~LED15;
    _T3IF = 0;
}

void _ISR _INT1Interrupt(void) {
    BUZZER = 1;
    _INT1IF = 0;
}

void main(void) {
    setup();

    /* setup timer-2 */
    T2CONbits.TCS = 0; // internal
    T2CONbits.TCKPS = 3; // x256
    PR2 = INST_FREQ / 256UL * 500UL / 1000UL; // 500ms
    _T2IF = 0;
    _T2IE = 1;
    T2CONbits.TON = 1;

    /* setup timer-3 */
    T3CONbits.TCS = 0; // internal
    T3CONbits.TCKPS = 3; // x256
    PR3 = INST_FREQ / 256UL * 1000UL / 1000UL; // 1s
    _T3IF = 0;
    _T3IE = 1;
    T3CONbits.TON = 1;

    /* setup external interruption */
    _INT1R = 0; // RB0 as source
    _INT1EP = 0; // detect rising edge
    _INT1IF = 0;
    _INT1IE = 1;

    /* loop */
    for (;;) {
        if (BUZZER) {
            delay_ms(100);
            BUZZER = 0;
        }
    }
}

#endif
