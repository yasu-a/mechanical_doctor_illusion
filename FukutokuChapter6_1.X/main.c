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

void serial_put(const char ch) {
    while (U1STAbits.UTXBF);
    U1TXREG = ch;
}

char serial_get(void) {
    uint16_t data;
    for (;;) {
        while (!U1STAbits.URXDA);
        unsigned char fail = 0;
        if (U1STAbits.OERR) {
            U1STAbits.OERR = 0;
            fail = 1;
        }
        if (U1STAbits.FERR) {
            data = U1RXREG;
            U1STAbits.FERR = 0;
            fail = 1;
        }
        if (fail) {
            continue;
        }
        data = U1RXREG;
        return data;
    }
}

/* 
 * Delay
 */

void delay_ms(const unsigned int ms) {
    static const uint64_t u64_inst_freq = INST_FREQ;
    const uint32_t inst = u64_inst_freq * (uint64_t) ms / 1000ULL;
    __delay32(inst);
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
}

/* 
 * (User)
 */

typedef volatile struct {
    unsigned int pressed;
    timestamp_t last_time_pressed_ms;
    unsigned int state;
    unsigned int count;
} button_state_s;

void button_set_pressed(button_state_s *bs) {
    bs->pressed = 1;
}

#define BTN_HOLD_TIME_MS 30

void button_process_count(
        button_state_s *bs,
        const timestamp_t current_time_ms) {
    unsigned int pressed = bs->pressed;
    if (bs->pressed) bs->pressed = 0;

    switch (bs->state) {
        case 0:
            if (pressed) {
                bs->state = 1;
                bs->count++;
            }
            break;
        case 1:
            bs->last_time_pressed_ms = current_time_ms;
            bs->state = 2;
            break;
        case 2:
            if (pressed) {
                bs->state = 1;
            } else {
                const timestamp_t elapsed_ms
                        = current_time_ms - bs->last_time_pressed_ms;
                if (elapsed_ms > BTN_HOLD_TIME_MS) {
                    bs->state = 0;
                }
            }
            break;
    }
}

static volatile struct state_s {
    unsigned int interval_12;
    unsigned int interval_13;
    unsigned int interval_14;
    button_state_s bs_0;
    button_state_s bs_1;
} state = {
    300, 200, 100,
    {0},
    {0},
};

void _ISR _T4Interrupt(void) {
    _T4IF = 0;

    /* led blink */
    timestamp_t current_time_millis = millis();

#define led_state(interval) ((current_time_millis * 2 / (interval)) & 1)

    _LATB12 = led_state(state.interval_12);
    _LATB13 = led_state(state.interval_13);
    _LATB14 = led_state(state.interval_14);

#undef led_state
}

void _ISR _T1Interrupt(void) {
    _T1IF = 0;

    /* button process */
    if (_RB0) button_set_pressed(&state.bs_0);
    if (_RB1) button_set_pressed(&state.bs_1);

    timestamp_t current_time_ms = millis();
    button_process_count(&state.bs_0, current_time_ms);
    button_process_count(&state.bs_1, current_time_ms);
}

void setup_interruption(void) {
    /* T4: interruption timer for led blink */
    T4CONbits.TON = 1;
    T4CONbits.TCS = 0; // internal signal source
    T4CONbits.TCKPS = 0; // x1
    PR4 = INST_FREQ * 1UL / 1000UL / 1UL; // 1ms
    _T4IF = 0;
    _T4IE = 1;

    /* T1: interruption timer for button process */
    T1CONbits.TON = 1;
    T1CONbits.TCS = 0; // internal signal source
    T1CONbits.TCKPS = 2; // x64
    PR1 = INST_FREQ * 10UL / 1000UL / 64UL; // 10ms;
    _T1IF = 0;
    _T1IE = 1;
}

typedef struct {
    unsigned int interval_12;
    unsigned int interval_13;
    unsigned int interval_14;
} command_s;

void command_decode(command_s *command, const char *buf) {
    /* parse serial string to command */

    command->interval_12 = (unsigned char) buf[0];
    command->interval_13 = (unsigned char) buf[1];
    command->interval_14 = (unsigned char) buf[2];
}

typedef struct {
    unsigned int count_0;
    unsigned int count_1;
} response_s;

void response_encode(response_s *response, char *buf) {
    /* parse response to serial string */

    buf[0] = response->count_0;
    buf[1] = response->count_1;
}

int main() {
    setup();

    char buf[32];
    for (;;) {
        /* read command from serial */
        for (unsigned int i = 0; i < 3; i++) {
            buf[i] = serial_get();
        }

        command_s command;
        command_decode(&command, buf);

        state.interval_12 = command.interval_12;
        state.interval_13 = command.interval_13;
        state.interval_14 = command.interval_14;

        /* write response to serial */
        response_s response;
        response.count_0 = state.bs_0.count;
        response.count_1 = state.bs_1.count;

        response_encode(&response, buf);

        for (unsigned int i = 0; i < 2; i++) {
            serial_put(buf[i]);
        }
    }
}
