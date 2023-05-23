/*
 * io.h
 *
 * Created: 11/09/2019
 * Author : Richard Tomlinson G4TGJ
 */ 
 
#ifndef IO_H
#define IO_H

#include <inttypes.h>

// Initialise all IO ports
void ioInit();

// Read the morse dot and dash paddles
bool ioReadDotPaddle();
bool ioReadDashPaddle();

// Read the rotary control and switch
void ioReadRotary( bool *pbA, bool *pbB, bool *pbSw );

// Read the left and right pushbuttons
bool ioReadLeftButton();
bool ioReadRightButton();

// Set the morse output high or low
void ioWriteMorseOutputHigh();
void ioWriteMorseOutputLow();

// Set the RX enable low or high
void ioWriteRXEnableLow();
void ioWriteRXEnableHigh();

// Switch the sidetone output on or off
void ioWriteSidetoneOn();
void ioWriteSidetoneOff();

// Switch a band relay output on or off
void ioWriteBandRelay( uint8_t relay, bool bOn );

#ifdef SOTA2
// Turn LEDs on or off
void ioWriteRightLED( bool bOn );
void ioWriteCentreLED( bool bOn );
void ioWriteLeftLED( bool bOn );
#endif

#ifdef VARIABLE_SIDETONE_VOLUME
void ioWriteSidetoneDutyCycle( uint8_t duty );
#endif

#endif //IO_H
