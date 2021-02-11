/*
 * io.c
 *
 * Created: 11/09/2019
 * Author : Richard Tomlinson G4TGJ
 */ 

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "io.h"

// Functions to read and write inputs and outputs
// This isolates the main logic from the I/O functions making it
// easy to change the hardware e.g. we have the option to
// connect several inputs to a single ADC pin


#ifdef VPORTC

// ATtiny 1-series

// Configure all the I/O we need
void ioInit()
{
    /* Set all pins to low power mode */

    for (uint8_t i = 0; i < 8; i++) {
        *((uint8_t *)&PORTA + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
    }

    for (uint8_t i = 0; i < 8; i++) {
        *((uint8_t *)&PORTB + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
    }

    for (uint8_t i = 0; i < 8; i++) {
        *((uint8_t *)&PORTC + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
    }

    // Set the input pins to inputs with pull ups
    ROTARY_ENCODER_A_DIR_REG &= ~(1 << ROTARY_ENCODER_A_PIN);
    ROTARY_ENCODER_A_PIN_CTRL |= (1 << PORT_PULLUPEN_bp);
    ROTARY_ENCODER_B_DIR_REG &= ~(1 << ROTARY_ENCODER_B_PIN);
    ROTARY_ENCODER_B_PIN_CTRL |= (1 << PORT_PULLUPEN_bp);

    MORSE_PADDLE_DOT_DIR_REG &= ~(1 << MORSE_PADDLE_DOT_PIN);
    MORSE_PADDLE_DOT_PIN_CTRL |= (1 << PORT_PULLUPEN_bp);
    MORSE_PADDLE_DASH_DIR_REG &= ~(1 << MORSE_PADDLE_DASH_PIN);
    MORSE_PADDLE_DASH_PIN_CTRL |= (1 << PORT_PULLUPEN_bp);

    // The three pushbuttons may be on an analogue input to save pins
#ifdef ANALOGUE_BUTTONS
    // Disable digital input buffer
    BUTTON_ADC_PINCTRL &= ~PORT_ISC_gm;
    BUTTON_ADC_PINCTRL |= PORT_ISC_INPUT_DISABLE_gc;

    // Enable pull-up resistor
    BUTTON_ADC_PINCTRL |= PORT_PULLUPEN_bm;

    // Set clock prescaler and voltage reference + sample capacitance
    // We divide the clock by the maximum because we have no need to sample
    // very often.
    BUTTON_ADC.CTRLC = ADC_SAMPCAP_bm | ADC_PRESC_DIV256_gc | ADC_REFSEL_VDDREF_gc;

    // Enable the ADC in 8 bit mode
    BUTTON_ADC.CTRLA = ADC_ENABLE_bm | ADC_RESSEL_8BIT_gc;

    // Select ADC channel
    BUTTON_ADC.MUXPOS = BUTTON_ADC_CHAN;

    // Enable FreeRun mode
    BUTTON_ADC.CTRLA |= ADC_FREERUN_bm;

    // Set delay and sample time to minimise sample rate
    BUTTON_ADC.CTRLD = ADC_SAMPDLY_gm;
    BUTTON_ADC.SAMPCTRL = ADC_SAMPLEN_gm;

    // Start the first conversion
    // As we are in free run mode it will just keep converting
    // and we can read it whenever we need the latest value
    BUTTON_ADC.COMMAND = ADC_STCONV_bm;

#else
    ROTARY_ENCODER_SW_DIR_REG &= ~(1 << ROTARY_ENCODER_SW_PIN);
    ROTARY_ENCODER_SW_PIN_CTRL |= (1 << PORT_PULLUPEN_bp);

    LEFT_DIR_REG &= ~(1 << LEFT_PIN);
    LEFT_PIN_CTRL |= (1 << PORT_PULLUPEN_bp);

    RIGHT_DIR_REG &= ~(1 << RIGHT_PIN);
    RIGHT_PIN_CTRL |= (1 << PORT_PULLUPEN_bp);
#endif

    // Set the output pins to outputs and turn off
    RELAY_0_OUTPUT_DIR_REG |= (1 << RELAY_0_OUTPUT_PIN);
    RELAY_0_OUTPUT_OUT_REG &= ~(1 << RELAY_0_OUTPUT_PIN);

    RELAY_1_OUTPUT_DIR_REG |= (1 << RELAY_1_OUTPUT_PIN);
    RELAY_1_OUTPUT_OUT_REG &= ~(1 << RELAY_1_OUTPUT_PIN);

    RELAY_2_OUTPUT_DIR_REG |= (1 << RELAY_2_OUTPUT_PIN);
    RELAY_2_OUTPUT_OUT_REG &= ~(1 << RELAY_2_OUTPUT_PIN);

    RELAY_3_OUTPUT_DIR_REG |= (1 << RELAY_3_OUTPUT_PIN);
    RELAY_3_OUTPUT_OUT_REG &= ~(1 << RELAY_3_OUTPUT_PIN);

    RELAY_4_OUTPUT_DIR_REG |= (1 << RELAY_4_OUTPUT_PIN);
    RELAY_4_OUTPUT_OUT_REG &= ~(1 << RELAY_4_OUTPUT_PIN);

    MORSE_OUTPUT_DIR_REG |= (1 << MORSE_OUTPUT_PIN);
    MORSE_OUTPUT_OUT_REG &= ~(1 << MORSE_OUTPUT_PIN);

    RX_ENABLE_DIR_REG |= (1 << RX_ENABLE_PIN);
    RX_ENABLE_OUT_REG |= (1 << RX_ENABLE_PIN);

    // Set up timer TCB1 to produce sidetone on PA3
    // We will get the clock from TCA0 (the millisecond timer) which we
    // divided down enough so that we can get an audible frequency
#define PERIOD (F_CPU/CLOCK_DIV/CW_FREQUENCY)
    TCB1.CCMPL = PERIOD;
    TCB1.CCMPH = PERIOD/50;
    TCB1.CTRLB = TCB_CNTMODE2_bm | TCB_CNTMODE1_bm | TCB_CNTMODE0_bm;
    TCB1.CTRLA = TCB_CLKSEL1_bm | TCB_ENABLE_bm;
    PORTA.PIN3CTRL &= PORT_PULLUPEN_bm;

    /* Insert nop for synchronization*/
    _NOP();
}

void ioReadRotary( bool *pbA, bool *pbB, bool *pbSw )
{
    *pbA  = !(ROTARY_ENCODER_A_IN_REG & (1 << ROTARY_ENCODER_A_PIN));
    *pbB  = !(ROTARY_ENCODER_B_IN_REG & (1 << ROTARY_ENCODER_B_PIN));
#ifdef ANALOGUE_BUTTONS
    *pbSw = ( (BUTTON_ADC.RES >= ROTARY_SW_MIN) && (BUTTON_ADC.RES <= ROTARY_SW_MAX) );
#else
    *pbSw = !(ROTARY_ENCODER_SW_IN_REG & (1 << ROTARY_ENCODER_SW_PIN));
#endif
}

// Read the left and right pushbuttons
bool ioReadLeftButton()
{
#ifdef ANALOGUE_BUTTONS
    return ( (BUTTON_ADC.RES >= LEFT_BUTTON_MIN) && (BUTTON_ADC.RES <= LEFT_BUTTON_MAX) );
#else
    return !(LEFT_IN_REG & (1 << LEFT_PIN));
#endif
}

bool ioReadRightButton()
{
#ifdef ANALOGUE_BUTTONS
    return ( (BUTTON_ADC.RES >= RIGHT_BUTTON_MIN) && (BUTTON_ADC.RES <= RIGHT_BUTTON_MAX) );
#else
    return !(RIGHT_IN_REG & (1 << RIGHT_PIN));
#endif
}

// Read the morse dot and dash paddles
bool ioReadDotPaddle()
{
    return !(MORSE_PADDLE_DOT_IN_REG & (1 << MORSE_PADDLE_DOT_PIN));
}

bool ioReadDashPaddle()
{
    return !(MORSE_PADDLE_DASH_IN_REG & (1 << MORSE_PADDLE_DASH_PIN));
}

// Set the morse output high or low
void ioWriteMorseOutputHigh()
{
    MORSE_OUTPUT_OUT_REG |= (1<<MORSE_OUTPUT_PIN);
}

void ioWriteMorseOutputLow()
{
    MORSE_OUTPUT_OUT_REG &= ~(1<<MORSE_OUTPUT_PIN);
}

// Set RX enable high or low
void ioWriteRXEnableHigh()
{
    RX_ENABLE_OUT_REG |= (1<<RX_ENABLE_PIN);
}

void ioWriteRXEnableLow()
{
    RX_ENABLE_OUT_REG &= ~(1<<RX_ENABLE_PIN);
}

// Switch the sidetone output on or off
void ioWriteSidetoneOn()
{
    // Enable the waveform output
    TCB1.CTRLB |= TCB_CCMPEN_bm;
}

void ioWriteSidetoneOff()
{
    // Disable the waveform output
    TCB1.CTRLB &= ~TCB_CCMPEN_bm;
}

// Map between relay number and the output register and pin
static const struct
{
    register8_t *outReg;
    uint8_t      pinVal;
}
relayMap[NUM_RELAYS] =
{
    { &RELAY_0_OUTPUT_OUT_REG, (1 << RELAY_0_OUTPUT_PIN) },
    { &RELAY_1_OUTPUT_OUT_REG, (1 << RELAY_1_OUTPUT_PIN) },
    { &RELAY_2_OUTPUT_OUT_REG, (1 << RELAY_2_OUTPUT_PIN) },
    { &RELAY_3_OUTPUT_OUT_REG, (1 << RELAY_3_OUTPUT_PIN) },
    { &RELAY_4_OUTPUT_OUT_REG, (1 << RELAY_4_OUTPUT_PIN) },
};

// Switch the band relay output on or off
void ioWriteBandRelay( uint8_t relay, bool bOn )
{
    if( relay < NUM_RELAYS )
    {
        if( bOn )
        {
            *relayMap[relay].outReg |= relayMap[relay].pinVal;
        }
        else
        {
            *relayMap[relay].outReg &= ~relayMap[relay].pinVal;
        }
    }
}

#else

#ifdef ROTARY_ANALOGUE

// We are using 2 ADCs.
enum eAdc
{
    ROTARY_A_ADC,
    ROTARY_B_ADC,
    NUM_ADCS
};

// The ADC we are currently converting
static enum eAdc currentADC = ROTARY_A_ADC;

// The latest data for each ADC
static uint16_t adcData[NUM_ADCS];

// ADC complete interrupt handler
ISR (ADC_vect)
{
    // Complete so read the result
    // Must read ADCL first
    adcData[currentADC] = ADCL;
    adcData[currentADC] |= ((ADCH&3)<<8);

    // Move to the next ADC
    if( currentADC == ROTARY_A_ADC )
    {
        currentADC = ROTARY_B_ADC;

        // Set the new input
        ADMUX = (1<<REFS0)|(0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0);
    }
    else
    {
        currentADC = ROTARY_A_ADC;

        // Set the new input
        ADMUX = (1<<REFS0)|(0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0);
    }

    // Start the next conversion
    ADCSRA |= (1<<ADSC);
}


// Read the ADC
static uint16_t ioReadADC( enum eAdc adc )
{
    uint16_t conversion;

    // Read the data ensuring the interrupt handler can't interfere
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        conversion = adcData[adc];
    }

#if 0
    static uint16_t prevConversion[8];

    if( abs((int)conversion - (int)prevConversion[adc]) > 5 )
    {
        char buf[TEXT_BUF_LEN];
        sprintf( buf, "%u:%u ", adc, conversion);
//        serialTXString( buf );
        displayText( MENU_LINE, buf, false );

        prevConversion[adc] = conversion;
    }
#endif

    return conversion;
}

void ioReadRotary( bool *pbA, bool *pbB, bool *pbSw )
{
    uint16_t adc;
    adc = ioReadADC( ROTARY_B_ADC );
    *pbB = ( ((adc >= ROTARY_B_MIN) && (adc <= ROTARY_B_MAX)) );

    adc = ioReadADC( ROTARY_A_ADC );
    *pbA = ( ((adc >= ROTARY_A_MIN) && (adc <= ROTARY_A_MAX)) );
    *pbSw = ( (adc >= ROTARY_SW_MIN) && (adc <= ROTARY_SW_MAX) );
}
#else
// Functions to read rotary control inputs
bool ioReadRotaryA()
{
    return !(ROTARY_ENCODER_A_PIN_REG & (1<<ROTARY_ENCODER_A_PIN));
}

bool ioReadRotaryB()
{
    return !(ROTARY_ENCODER_B_PIN_REG & (1<<ROTARY_ENCODER_B_PIN));
}

bool ioReadRotarySW()
{
    return !(ROTARY_ENCODER_SW_PIN_REG & (1<<ROTARY_ENCODER_SW_PIN));
}
#endif

// Functions to read morse paddle inputs
bool ioReadDotPaddle()
{
    return !(MORSE_PADDLE_DOT_PIN_REG & (1<<MORSE_PADDLE_DOT_PIN));
}

bool ioReadDashPaddle()
{
    return !(MORSE_PADDLE_DASH_PIN_REG & (1<<MORSE_PADDLE_DASH_PIN));
}

// Set the morse output high
void ioWriteMorseOutputHigh()
{
    MORSE_OUTPUT_PORT_REG |= (1<<MORSE_OUTPUT_PIN);
}

// Set the morse output low
void ioWriteMorseOutputLow()
{
    MORSE_OUTPUT_PORT_REG &= ~(1<<MORSE_OUTPUT_PIN);
}

// Set RX enable low
void ioWriteRXEnableLow()
{
    RX_ENABLE_PORT_REG &= ~(1<<RX_ENABLE_PIN);
}

// Set RX enable high
void ioWriteRXEnableHigh()
{
    RX_ENABLE_PORT_REG |= (1<<RX_ENABLE_PIN);
}

// Switch the sidetone output on
void ioWriteSidetoneOn()
{
    TCCR0A |= (1<<COM0A0);
}

// Switch the sidetone output off
void ioWriteSidetoneOff()
{
    TCCR0A &= ~(1<<COM0A0);
}

// Switch the band relay output on
void ioWriteBandRelayOn()
{
    RELAY_OUTPUT_PORT_REG |= (1<<RELAY_OUTPUT_PIN);
}

// Switch the band relay output off
void ioWriteBandRelayOff()
{
    RELAY_OUTPUT_PORT_REG &= ~(1<<RELAY_OUTPUT_PIN);
}

void ioInit()
{
	// Initialise morse and LED outputs
    MORSE_OUTPUT_DDR_REG |= (1<<MORSE_OUTPUT_PIN);
    RX_ENABLE_DDR_REG    |= (1<<RX_ENABLE_PIN);
    RELAY_OUTPUT_DDR_REG   |= (1<<RELAY_OUTPUT_PIN);

    // Turn off the band relay
    RELAY_OUTPUT_PORT_REG &= ~(1<<RELAY_OUTPUT_PIN);

    // Unmute the RX
    RX_ENABLE_PORT_REG |= (1<<RX_ENABLE_PIN);
    
    // Paddle dot and dash pins as inputs with pull-ups
    MORSE_PADDLE_DOT_PORT_REG |= (1<<MORSE_PADDLE_DOT_PIN);
    MORSE_PADDLE_DASH_PIN_REG |= (1<<MORSE_PADDLE_DASH_PIN);

#ifdef ROTARY_ANALOGUE
    // Initialise the ADC
    // Use internal AVCC as reference
    // Start with ADC7
    ADMUX = (1<<REFS0)|(0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0);

    // Enable the ADC, enable interrupts and set the prescaler for the
    // correct ADC clock rate
    ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

    // Start the first conversion
    ADCSRA |= (1<<ADSC);
#else
    // Initialise rotary encoder pins with pull-ups
    ROTARY_ENCODER_A_PORT_REG  |= (1<<ROTARY_ENCODER_A_PIN);
    ROTARY_ENCODER_B_PORT_REG  |= (1<<ROTARY_ENCODER_B_PIN);
    ROTARY_ENCODER_SW_PORT_REG |= (1<<ROTARY_ENCODER_SW_PIN);
#endif

    // Setup timer 0 to produce sidetone
    // Tone only output on key down
    DDRD |= (1<<PD6);
    TCCR0A = ((1<<WGM01) | (0<<WGM00));
    TCCR0B = ((1<<CS02));
    OCR0A = (F_CPU / 256 / CW_FREQUENCY / 2);

    /* Insert nop for synchronization*/
    _NOP();
}

#endif