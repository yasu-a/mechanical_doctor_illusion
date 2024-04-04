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

#define BUTTON0 (PORTBbits.RB0) 
#define BUTTON1 (PORTBbits.RB1) 

void setup(void) {
    setup_clock();

    /* setup timers */
    //    T1CONbits.TON = 1; // enable timer 2
    //    T1CONbits.TCS = 0; // internal signal source
    //    T1CONbits.TCKPS = 1; // prescale x8
    //#define timer1_pr(us) (INST_FREQ * (uint32_t)us / 1000UL / 1000UL / 8UL)
    //    PR1 = timer1_pr(100);
    //
    //    T2CONbits.TON = 1;
    //    T2CONbits.TCS = 1; // external signal source
    //    T2CONbits.TCKPS = 0; // x1
    //    _T2CKR = 0; // set RB0 as signal source
    //    TMR2 = 0;

    /* setup I/Os */
    TRISA = 0x0003; // 1: input, 0: output
    TRISB = 0x0103;
    AD1PCFGL = 0xfffc; // use RB1, RB0 as analong inputs
}

#define PRACTICE_NUMBER 4

void main(void) {
    setup();

#if PRACTICE_NUMBER == 1
    /* setup timer-4 */
    T4CONbits.TON = 1;
    T4CONbits.TCS = 1; // external
    T4CONbits.TCKPS = 0; // x1
    _T4CKR = 0; // RB0 as signal source

    /* loop */
    TMR4 = 0;
    for (;;) {
        write_led(TMR4);
    }
#elif PRACTICE_NUMBER == 2
    /* setup timer-5 */
    T5CONbits.TON = 1;
    T5CONbits.TCS = 0; // internal
    T5CONbits.TCKPS = 3; // x256
    PR5 = INST_FREQ / 256UL; // counts per second

    /* loop */
    TMR5 = 0;
    for (;;) {
        if (_T5IF) {
            LED12 = ~LED12;
            _T5IF = 0;
        }
    }
#elif PRACTICE_NUMBER == 3
    /* timer-1: interval 42ms */
    T1CONbits.TON = 1; // enable timer
    T1CONbits.TCS = 0; // internal signal source
    T1CONbits.TCKPS = 2; // prescale x64
    PR1 = INST_FREQ * 42UL / 1000UL / 64UL; // 42ms

    /* timer-2: hum count */
    T2CONbits.TON = 1; // enable timer
    T2CONbits.TCS = 1; // external signal source
    T2CONbits.TCKPS = 0; // prescale x1
    _T2CKR = 0; // RB0 as signal source

    /* timer-4/5: interval 2s */
    T4CONbits.TON = 0; // disable timer
    T4CONbits.TCS = 0; // internal signal source
    T4CONbits.TCKPS = 2; // prescale x64
    T4CONbits.T32 = 1; // 32bit counter
    uint32_t u32_pr = INST_FREQ * 2UL / 64UL; // 2s
    PR4 = u32_pr & 0xffff;
    PR5 = (u32_pr >> 16) & 0xffff;

    /* 32bit timer testing */
    //    T4CONbits.TON = 1;
    //    for (;;) {
    //        write_led(TMR5);
    //    }

    /* loop */
    TMR2 = 0;
    for (;;) {
        if (_T1IF) {
            _T1IF = 0;
            LED12 = TMR2 >= 2;
            if (LED12) {
                if (!T4CONbits.TON) {
                    TMR5HLD = 0;
                    TMR4 = 0;
                    T4CONbits.TON = 1;
                } else {
                    if (_T5IF) {
                        _T5IF = 0;
                        LED15 = 1;
                    }
                }
            } else {
                LED12 = LED15 = 0;
                T4CONbits.TON = 0;
            }
            TMR2 = 0;
        }
    }
#elif PRACTICE_NUMBER == 4
    /* timer-4/5: interval 2s */
    T4CONbits.TON = 1; // disable timer
    T4CONbits.TCS = 0; // internal signal source
    T4CONbits.TCKPS = 2; // prescale x64
    T4CONbits.T32 = 1; // 32bit counter
    uint32_t u32_pr = INST_FREQ * 2UL / 64UL; // 2s
    PR4 = u32_pr & 0xffff;
    PR5 = (u32_pr >> 16) & 0xffff;

    /* loop */
    for (;;) {
        if (_T5IF) {
            _T5IF = 0;
            LED12 = ~LED12;
        }
    }
#endif
}