/*
 * main.h
 *
 * Created: 31/01/2020 14:23:53
 * Author : Richard Tomlinson G4TGJ
 */ 


#ifndef MAIN_H_
#define MAIN_H_

extern uint32_t rxFreq;

#define NUM_VFOS 2
#define VFO_A 0
#define VFO_B 1

// CAT driver
void     setVFOFrequency( uint8_t vfo, uint32_t freq );
uint32_t getVFOFreq( uint8_t vfo );
uint32_t getCurrentVFOFreq();
uint32_t getOtherVFOFreq();
int16_t  getCurrentVFOOffset();
bool     getCurrentVFORIT();
bool     getCurrentVFOXIT();
int16_t  getOtherVFOOffset();
bool     getOtherVFORIT();
bool     getOtherVFOXIT();
uint8_t  getCurrentVFO();
void     setCurrentVFO( uint8_t vfo );
void     setCurrentVFORIT( bool bRIT );
bool     getVFOSplit();
void     setVFOSplit( bool bSplit );
bool     getTransmitting();
void     vfoSwap();
void     vfoEqual();
void     setCurrentVFOOffset( int16_t rit );
void     setCWReverse( bool bCWReverse );

// Morse driver
// Display a character on the screen as sent or received (if implemented)
void     displayMorse( char *text );

void     keyDown( bool bDown );

#endif /* MAIN_H_ */