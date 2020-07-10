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

// Set the morse output high
// Functions to read morse paddle inputs
bool ioReadDotPaddle()
{
    return !(MORSE_PADDLE_DOT_PIN_REG & (1<<MORSE_PADDLE_DOT_PIN));
}

bool ioReadDashPaddle()
{
    return !(MORSE_PADDLE_DASH_PIN_REG & (1<<MORSE_PADDLE_DASH_PIN));
}

void ioWriteMorseOutputHigh()
{
    MORSE_OUTPUT_PORT_REG |= (1<<MORSE_OUTPUT_PIN);
}

// Set the morse output low
void ioWriteMorseOutputLow()
{
    MORSE_OUTPUT_PORT_REG &= ~(1<<MORSE_OUTPUT_PIN);
}

// Set the TX output low
void ioWriteTXOutputLow()
{
    TX_OUTPUT_PORT_REG &= ~(1<<TX_OUTPUT_PIN);
}

// Set the TX output high
void ioWriteTXOutputHigh()
{
    TX_OUTPUT_PORT_REG |= (1<<TX_OUTPUT_PIN);
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

void ioConfigure()
{
	// Initialise morse and LED outputs
    MORSE_OUTPUT_DDR_REG |= (1<<MORSE_OUTPUT_PIN);
    TX_OUTPUT_DDR_REG    |= (1<<TX_OUTPUT_PIN);
    RELAY_OUTPUT_DDR_REG   |= (1<<RELAY_OUTPUT_PIN);

    // Turn off the band relay
    RELAY_OUTPUT_PORT_REG &= ~(1<<RELAY_OUTPUT_PIN);

    // Unmute the RX
    TX_OUTPUT_PORT_REG |= (1<<TX_OUTPUT_PIN);
    
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

