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

// Set the morse output high or low
void ioWriteMorseOutputHigh();
void ioWriteMorseOutputLow();

// Set the TX output low or high
void ioWriteTXOutputLow();
void ioWriteTXOutputHigh();

// Switch the sidetone output on or off
void ioWriteSidetoneOn();
void ioWriteSidetoneOff();

// Switch the band relay output on or off
void ioWriteBandRelayOn();
void ioWriteBandRelayOff();


#endif //IO_H
