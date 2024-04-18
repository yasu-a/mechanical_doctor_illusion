#include <xc.h>
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

uint8_t serial_get_byte_no_block(uint8_t default_value) {
    if (U1STAbits.URXDA) {
        uint16_t value;
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
            return default_value;
        }
        value = U1RXREG;
        return value & 0xff;
    } else {
        return default_value;
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
    _RP12R = 18; // OC1 -> RP12

    OC2CONbits.OCTSEL = 0; // Timer2 -> OC2
    OC2CONbits.OCM = 6; // PWM mode
    OC2RS = 0;
    _RP13R = 19; // OC2 -> RP13

    OC3CONbits.OCTSEL = 0; // Timer2 -> OC3
    OC3CONbits.OCM = 6; // PWM mode
    OC3RS = 0;
    _RP14R = 20; // OC3 -> RP14

    OC4CONbits.OCTSEL = 0; // Timer2 -> OC3
    OC4CONbits.OCM = 6; // PWM mode
    OC4RS = 0;
    _RP15R = 21; // OC4 -> RP15

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

int write_pwm(int i, float duty_ratio) {
    uint16_t ocxrs = (uint16_t) ((float) PR2 * duty_ratio);
    switch (i) {
        case 1:
            OC1RS = ocxrs;
            break;
        case 2:
            OC2RS = ocxrs;
            break;
        case 3:
            OC3RS = ocxrs;
            break;
        case 4:
            OC4RS = ocxrs;
            break;
        default:
            return -1;
    }
    return 0;
}

float serial_get_fixed_float() {
    uint16_t value = (uint16_t) serial_get_byte() << 8 | (uint16_t) serial_get_byte();
    return (float) value / (float) 65535;
}

int main(void) {
    setup();

    printf("HELLO!\n");

    for (;;) {
        delay_ms(100);
        if (serial_get_byte() != 0xaa) continue;
        if (serial_get_byte() != 0x55) continue;
        int index = (int) serial_get_byte();
        float duty_ratio = serial_get_fixed_float();
        int result = write_pwm(index, duty_ratio);
        if (result < 0) {
            printf("NG\n");
        } else {
            printf("OK %d %f\n", index, duty_ratio);
        }
    }
}