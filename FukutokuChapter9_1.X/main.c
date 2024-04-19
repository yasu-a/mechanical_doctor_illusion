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

static uint32_t t2_interval_us;
static uint32_t t3_per_ms;

void setup(void) {
    setup_clock();
    setup_serial();

    /* setup I/Os */
    _U1RXR = 9; // UART input
    _RP8R = 3; // UART output
    TRISA = 0x001f; // 1: input, 0: output
    TRISB = 0x0203;

    /* setup timer for PWM generator */
    T2CONbits.TCS = 0; // internal
    T2CONbits.TCKPS = 1; // x8
    t2_interval_us = 10UL * 1000UL;
    PR2 = INST_FREQ * t2_interval_us / 1000UL / 1000UL / 8UL;
    T2CONbits.TON = 1;

    /* setup tiemr for capturing duty ratio */
    T3CONbits.TCS = 0; // internal
    T3CONbits.TCKPS = 1; // x8
    t3_per_ms = INST_FREQ / 1000UL / 8UL;
    PR3 = 0xffff;
    T3CONbits.TON = 1;

    /* setup output compare */
    OC1CONbits.OCTSEL = 0; // Timer2 -> OC1
    OC1CONbits.OCM = 6; // PWM mode
    OC1RS = 0;
    _RP12R = 18; // OC1 -> RP12

    OC2CONbits.OCTSEL = 0; // Timer2 -> OC2
    OC2CONbits.OCM = 6; // PWM mode
    OC2RS = 0;
    _RP13R = 19; // OC2 -> RP13

    /* setup input capture */
    IC2CONbits.ICTMR = 0; // Timer3 -> timer source
    IC2CONbits.ICI = 0; // interrupt every event
    IC2CONbits.ICM = 3; // rising edge
    _IC2R = 12; // RP12 -> IC2
    _IC2IF = 0;
    _IC2IE = 1;
}

/* 
 * (User)
 */

volatile uint16_t t_duty = 0;

void _ISR _IC2Interrupt(void) {
    static uint16_t t_rise = 0;
    static uint16_t t_fall = 0;
    if (IC2CONbits.ICM == 3) { // rising edge
        t_rise = IC2BUF;
        IC2CONbits.ICM = 2; // falling edge
    } else { // falling edge
        t_fall = IC2BUF;
        IC2CONbits.ICM = 3; // rising edge
        if (t_rise > t_fall) {
            t_duty = PR3 - t_rise + t_fall + 1; // XXX: why plus 1???
        } else {
            t_duty = t_fall - t_rise;
        }
    }
    _LATB15 = ~_LATB15;
    _IC2IF = 0;
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

int main(void) {
    setup();

    printf("HELLO!\n");
    write_pwm(1, 0.123456);

    for (;;) {
        uint32_t t_duty_in_us = (uint32_t) t_duty * 1000UL / t3_per_ms;
        printf("PT %u %lu %lu[us]/%lu[us]=%.1f[%]\n", t_duty, t3_per_ms, t_duty_in_us, t2_interval_us, (float) t_duty_in_us / (float) t2_interval_us * 100.0f);
        delay_ms(100);
//        if (serial_get_byte() != 0xaa) continue;
//        if (serial_get_byte() != 0x55) continue;
//        int index = (int) serial_get_byte();
//        float duty_ratio = serial_get_fixed_float();
//        int result = write_pwm(index, duty_ratio);
//        if (result < 0) {
//            printf("NG\n");
//        } else {
//            printf("OK %d %f\n", index, duty_ratio);
//        }
    }
}