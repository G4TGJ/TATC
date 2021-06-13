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
#include <ctype.h>

#include "config.h"
#include "eeprom.h"
#include "nvram.h"

#ifdef SOTA2

// Magic numbers used to help verify the data is correct
// ASCII "T2S " in little endian format
#define MAGIC 0x20533254

// Data format:
// T2S ffffffff x yy
//
// Always starts with T2S
//
// ffffffff is the xtal frequency
// x is A for Iambic A, B for Iambic B or U for Ultimatic
// yy is the morse speed in wpm
//
// For example:
// T2S 27000123 A 18
//
// If the format is incorrect or the slow and fast speeds are outside
// the min and max limits defined in config.h then the default values
// from config.h are used.

// Cached version of the NVRAM - read from the EEPROM at boot time
struct __attribute__ ((packed)) sNvramCache
{
    uint32_t magic;         // Magic number to check data is valid
    char    xtal_freq[8];   // Xtal frequency
    char    space1;         // Expect this to be a space
    char    keyer_mode;     // Character for the keyer mode
    char    space2;         // Also expect this to be a space
    char    wpm[2];         // Morse speed in wpm
};

// Validated xtal frequency
static uint32_t xtalFreq;

// Validated keyer mode and morse speed as read from the EEPROM
static enum eMorseKeyerMode keyerMode;
static uint8_t morseSpeed;

// Convert n characters into a number
static uint32_t convertNum( char *num, uint8_t n )
{
    uint8_t i;
    uint32_t result = 0;
    
    for( i = 0 ; i < n ; i++ )
    {
        // First check the characters is a decimal digit
        if( isdigit(num[i]) )
        {
            // Convert from ASCII to decimal and shift into the running total
            result = (num[i]-'0') + (result * 10);
        }
        else
        {
            // Not a digit so return 0
            result = 0;
            break;
        }
    }

    return result;
}

// Initialise the NVRAM - read it in and check valid.
// Must be called before any operations
void nvramInit()
{
    struct sNvramCache nvram_cache;

    bool bValid = false;

    // Read from the EEPROM into the NVRAM cache
    for( int i = 0 ; i < sizeof( nvram_cache ) ; i++ )
    {
        ((uint8_t *) &nvram_cache)[i] = eepromRead(i);
    }
    
    // Check the magic numbers and spaces are correct
    if( (nvram_cache.magic == MAGIC) &&
        (nvram_cache.space1 == ' ') &&
        (nvram_cache.space2 == ' ') )
    {
        bValid = true;

        // Get the xtal frequency and check it is within range
        xtalFreq = convertNum( nvram_cache.xtal_freq, 8 );
        if( (xtalFreq < MIN_XTAL_FREQUENCY) || (xtalFreq > MAX_XTAL_FREQUENCY) )
        {
            bValid = false;
        }

        // Get the keyer mode
        if( nvram_cache.keyer_mode == 'A' )
        {
            keyerMode = morseKeyerIambicA;
        }
        else if( nvram_cache.keyer_mode == 'B' )
        {
            keyerMode = morseKeyerIambicB;
        }
        else if( nvram_cache.keyer_mode == 'U' )
        {
            keyerMode = morseKeyerUltimatic;
        }
        else
        {
            // Not valid
            bValid = false;
        }

        // Get the morse speed
        // First check both characters are decimal digits
        if( isdigit(nvram_cache.wpm[0]) && isdigit(nvram_cache.wpm[1]) )
        {
            // Convert from ASCII to decimal
            morseSpeed = (nvram_cache.wpm[0]-'0')*10 + (nvram_cache.wpm[1]-'0');

            // Check it is within range
            if( (morseSpeed < MIN_MORSE_WPM) || (morseSpeed > MAX_MORSE_WPM) )
            {
                bValid = false;
            }
        }
        else
        {
            bValid = false;
        }
    }

    // If any of it wasn't valid then set the defaults
    if( !bValid )
    {
        xtalFreq = DEFAULT_XTAL_FREQ;
        keyerMode = DEFAULT_KEYER_MODE;
        morseSpeed = DEFAULT_MORSE_WPM;
    }
}

// Functions to read and write parameters in the NVRAM
// Read is actually already read and validated
uint32_t nvramReadXtalFreq()
{
    return xtalFreq;
}

uint8_t nvramReadWpm()
{
    return morseSpeed;
}

enum eMorseKeyerMode nvramReadMorseKeyerMode()
{
    return keyerMode;
}

void nvramWriteWpm( uint8_t wpm )
{
}

void nvramWriteXtalFreq( uint32_t freq )
{
}

void nvramWriteMorseKeyerMode( enum eMorseKeyerMode keyer_mode )
{
}

void nvramWriteBand( uint8_t band )
{
}

void nvramWriteCWReverse( bool bCWReverse )
{
}

uint8_t nvramReadBand()
{
    return 0;
}

uint8_t nvramReadCWReverse()
{
    return false;
}


#else

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

#endif