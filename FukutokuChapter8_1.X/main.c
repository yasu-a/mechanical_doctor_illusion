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

#define SERIAL_BAUDRATE 115200

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

    /* setup timer */
    T2CONbits.TCS = 0; // internal
    T2CONbits.TCKPS = 1; // x8
    PR2 = INST_FREQ * 10UL / 1000UL / 8UL;
    T2CONbits.TON = 1;

    /* setup output compare */
    OC1CONbits.OCTSEL = 0; // Timer2 -> OC1
    OC1CONbits.OCM = 6; // PWM mode
    OC1RS = 0;
    _RP12R = 18;  // OC1 -> RP12
    _RP13R = 18;  // OC1 -> RP13
    _RP14R = 18;  // OC1 -> RP13
    _RP15R = 18;  // OC1 -> RP15
    
    // ADC with single channel, manual sampling, auto triggering conversion,
    //          and 10bit resolution
    AD1PCFGL = 0xFFFC; // mark AN0, AN1 as analog input
    AD1CON1bits.SSRC = 0b111; // finish sampling by internal counter
    AD1CON2 = 0; // default configuration
    AD1CON3bits.ADCS = 2; // T_ad = (ADCS + 1) * T_cy = T_cy * 3 (12bit@M=33, 30.401250MHz)
    AD1CON3bits.SAMC = 3; // T_samp = T_ad * SAMC = T_ad * 3 (12bit@M=33, 30.401250MHz)
    AD1CHS0bits.CH0SA = 0; // input channel selection
    AD1CON1bits.ADON = 1; // enable ADC
}

/* 
 * (User)
 */

uint16_t analog_read(int index) {
    switch (index) {
        case 0:
            AD1CHS0bits.CH0SA = 0;
            break;
        case 1:
            AD1CHS0bits.CH0SA = 1;
            break;
    }
    AD1CON1bits.SAMP = 1;
    while (!AD1CON1bits.DONE);
    AD1CON1bits.DONE = 0;
    const uint16_t value = ADC1BUF0;
    return value;
}

void write_pwm(float duty_ratio) {
    OC1RS = (uint16_t)((float)PR2 * duty_ratio);
}

void main(void) {
    setup();
    
    write_pwm(0);

    for (;;) {
        float duty_ratio = (float)analog_read(0) / 1024.0f;
        duty_ratio += (float)analog_read(1) / 1024.0f / 1000.0f;
        write_pwm(duty_ratio);
        printf("PR2=%u OC1RS=%u %d %d\n", (unsigned int)PR2, (unsigned int)OC1RS, (unsigned int)analog_read(0), (unsigned int)analog_read(1));
        delay_ms(100);
    }
}