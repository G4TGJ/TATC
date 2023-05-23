/*
 * nvram.h
 *
 * Created: 07/08/2019
 * Author : Richard Tomlinson G4TGJ
 */ 
 

#ifndef NVRAM_H
#define NVRAM_H

#include <inttypes.h>
#include "morse.h"

// Backlight mode
enum eBacklightMode
{
    backlightOff = 0,
    backlightOn,
    backlightAuto,
    NUM_BACKLIGHT_MODES
};

void nvramInit();

uint8_t nvramReadWpm();
void nvramWriteWpm( uint8_t wpm );

uint32_t nvramReadXtalFreq();
void nvramWriteXtalFreq( uint32_t freq );

uint32_t nvramReadBFOFreq();
void nvramWriteBFOFreq( uint32_t freq );

enum eMorseKeyerMode nvramReadMorseKeyerMode();
void nvramWriteMorseKeyerMode( enum eMorseKeyerMode );

uint8_t nvramReadBand();
void nvramWriteBand( uint8_t band );

uint8_t nvramReadCWReverse();
void nvramWriteCWReverse( bool bCWReverse );

enum eBacklightMode nvramReadBacklighMode();
void nvramWriteBacklightMode( enum eBacklightMode );

uint8_t nvramReadSidetoneVolume();
void nvramWriteSidetoneVolume( uint8_t volume );

#endif //NVRAM_H
