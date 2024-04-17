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
 * Delay
 */

void delay_ms(const unsigned int ms) {
    static const uint64_t u64_inst_freq = INST_FREQ;
    const uint32_t n_inst = u64_inst_freq * (uint64_t) ms / 1000ULL;
    __delay32(n_inst);
}

/* 
 * Current Time Util
 */

#define __ENABLE_CURRENT_TIME__

typedef uint32_t timestamp_t;

void setup_ct(void) {
#ifdef __ENABLE_CURRENT_TIME__
    T2CONbits.TON = 1;
    T2CONbits.TCS = 0; // internal
    T2CONbits.TCKPS = 0; // x1
    T2CONbits.T32 = 1;
    PR2 = 0xffff;
    PR3 = 0xffff;
#endif /* __ENABLE_CURRENT_TIME__ */
}

timestamp_t micros(void) {
#ifdef __ENABLE_CURRENT_TIME__
    uint64_t u64_tmr = 0;
    u64_tmr |= (uint64_t) TMR2 << 0;
    u64_tmr |= (uint64_t) TMR3HLD << 16;

    return (timestamp_t) (u64_tmr * 1000ULL * 1000ULL / INST_FREQ);
#else
    return 0;
#endif /* __ENABLE_CURRENT_TIME__ */
}

timestamp_t millis(void) {
    return micros() / 1000UL;
}

/* 
 * Setup
 */

void setup_interruption(void);
void setup_adc(void);

void setup(void) {
    setup_clock();
    setup_serial();
    setup_ct();

    /* setup I/Os */
    _U1RXR = 9; // UART input
    _RP8R = 3; // UART output
    TRISA = 0x001f; // 1: input, 0: output
    TRISB = 0x0203;
    AD1PCFGL = 0xfffc; // use RB1, RB0 as analong inputs

    setup_interruption();
    setup_adc();
}

/* 
 * (User)
 */
struct {
    uint16_t interval_ms;
} state = {
    100,
};

void _ISR _T5Interrupt(void) {
    _T5IF = 0;
}

void set_counter_thresh(uint16_t interval_ms) {
    uint32_t value = INST_FREQ * (uint64_t) interval_ms / 2ULL / 1000ULL;
    PR4 = value & 0xffff;
    value >>= 16;
    PR5 = value & 0xffff;
}

void update_interval() {
    T4CONbits.TON = 0;
    _T5IE = 0;

    TMR5HLD = 0;
    TMR4 = 0;
    set_counter_thresh(state.interval_ms);

    _T5IE = 1;
    T4CONbits.TON = 1;
}

void setup_interruption(void) {
}

void setup_adc(void) {
    // ADC with single channel, manual sampling, auto triggering conversion,
    //          and 12bit resolution
    AD1PCFGL = 0xFFFC; // mark AN0, AN1 as analog input
    AD1CON1bits.SSRC = 0b111; // finish sampling by internal counter
    AD1CON1bits.AD12B = 1;
    AD1CON2 = 0; // default configuration
    AD1CON3bits.ADCS = 2; // T_ad = (ADCS + 1) * T_cy = T_cy * 3 (12bit@M=33, 30.401250MHz)
    AD1CON3bits.SAMC = 3; // T_samp = T_ad * SAMC = T_ad * 3 (12bit@M=33, 30.401250MHz)
    AD1CHS0bits.CH0SA = 0; // input channel selection
    AD1CON1bits.ADON = 1; // enable ADC
}

int main() {
    setup();

    for (;;) {
        AD1CON1bits.SAMP = 1;
        while (!AD1CON1bits.DONE);
        AD1CON1bits.DONE = 0;
        const uint16_t data = ADC1BUF0;
        LATB = (data << 4) & 0xF000;
    }
}
