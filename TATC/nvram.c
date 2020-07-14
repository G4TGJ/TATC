/*
 * nvram.c
 * 
 * Maintains a non-volatile memory area which
 * is stored in the EEPROM.
 *
 * Created: 07/08/2019
 * Author : Richard Tomlinson G4TGJ
 */ 
 
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "eeprom.h"
#include "nvram.h"

// Magic number used to help verify the data is correct
#define MAGIC 0x8402

// Cached version of the NVRAM - read from the EEPROM at boot time
static struct
{
    uint16_t magic;                         // Magic number to check data is valid
    uint32_t xtal_freq;                     // Crystal frequency
    uint8_t  wpm;                           // Morse WPM
    enum eMorseKeyerMode morse_keyer_mode;  // Morse keyer mode
    uint8_t band;                           // Frequency band
    bool bCWReverse;                        // True if in CW-Reverse
    uint16_t crc;                           // CRC to check that the data is valid
} nvram_cache;

// A simple checksum. Much smaller than a proper CRC.
static uint16_t checksum( const uint8_t *input_str, size_t num_bytes )
{
    uint16_t sum = 0;
    for( int i = 0 ; i < num_bytes ; i++ )
    {
        sum += input_str[i];
    }
    return sum;
}

// Calculate the CRC for the NVRAM cache - do not include the CRC itself
static uint16_t calc_crc()
{
    // Use a checksum instead of a proper CRC to save program and data space
    return checksum( (const uint8_t *) &nvram_cache, sizeof(nvram_cache) - sizeof(nvram_cache.crc) );
}

// Update the eeprom from the cache. Calculate the CRC and only
// write any changed bytes
static void nvramUpdate()
{
    nvram_cache.crc = calc_crc();
    
    // Only write back changed bytes to minimise EEPROM wear
    for( int i = 0 ; i < sizeof( nvram_cache ) ; i++ )
    {
        if( ((uint8_t *) &nvram_cache)[i] != eepromRead(i) )
        {
            eepromWrite( i, ((uint8_t *) &nvram_cache)[i]);
        }
    }
}

// Initialise the NVRAM - read it in and check valid.
// Must be called before any operations
void nvramInit()
{
    // Read from the EEPROM into the NVRAM cache
    for( int i = 0 ; i < sizeof( nvram_cache ) ; i++ )
    {
        ((uint8_t *) &nvram_cache)[i] = eepromRead(i);
    }
    
    // Check the CRC16 and magic numbers are correct
    if( calc_crc() != nvram_cache.crc ||
        nvram_cache.magic != MAGIC)
    {
        // CRC doesn't match so set the default values
        nvram_cache.wpm = DEFAULT_MORSE_WPM;
        nvram_cache.xtal_freq = DEFAULT_XTAL_FREQ;
        nvram_cache.morse_keyer_mode = DEFAULT_KEYER_MODE;
        nvram_cache.band = DEFAULT_BAND;
        nvram_cache.bCWReverse = DEFAULT_CWREVERSE;
        nvram_cache.magic = MAGIC;
        
        // Calculate the CRC and write to the EEPROM
        nvramUpdate();
    }
}

// Functions to read and write parameters in the NVRAM
// Read is done directly from the cache
// Writing updates the CRC and writes back changed bytes.
uint8_t nvramReadWpm()
{
    return nvram_cache.wpm;
}

void nvramWriteWpm( uint8_t wpm )
{
    nvram_cache.wpm = wpm;
    nvramUpdate();
}

uint32_t nvramReadXtalFreq()
{
    return nvram_cache.xtal_freq;
}

void nvramWriteXtalFreq( uint32_t freq )
{
    nvram_cache.xtal_freq = freq;
    nvramUpdate();
}

enum eMorseKeyerMode nvramReadMorseKeyerMode()
{
    return nvram_cache.morse_keyer_mode;
}

void nvramWriteMorseKeyerMode( enum eMorseKeyerMode keyer_mode )
{
    nvram_cache.morse_keyer_mode = keyer_mode;
    nvramUpdate();
}

uint8_t nvramReadBand()
{
    return nvram_cache.band;
}

void nvramWriteBand( uint8_t band )
{
    nvram_cache.band = band;
    nvramUpdate();
}

uint8_t nvramReadCWReverse()
{
    return nvram_cache.bCWReverse;
}

void nvramWriteCWReverse( bool bCWReverse )
{
    nvram_cache.bCWReverse = bCWReverse;
    nvramUpdate();
}