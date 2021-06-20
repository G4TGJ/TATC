/*
 * main.c
 *
 * Created: 30/11/2018 21:26:21
 * Author : Richard Tomlinson G4TGJ
 */ 

//#define DISABLE_LCD

#include <util/delay_basic.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "main.h"
#include "io.h"
#include "osc.h"
#include "millis.h"
#include "nvram.h"
#include "display.h"
#include "morse.h"
#include "cat.h"
#include "rotary.h"
#include "pushbutton.h"

#ifndef SOTA2
// Menu functions
static bool menuVFOBand( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuVFOMode( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuBreakIn( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuTestRXMute( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuSidetone( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuRXClock( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuUnmuteDelay( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuMuteDelay( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuTXDelay( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuTXClock( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuTXOut( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuXtalFreq( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );
static bool menuKeyerMode( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );

// Menu structure arrays

struct sMenuItem
{
    char *text;
    bool (*func)(bool, bool, bool, bool, bool, bool, bool, bool);
};

#define NUM_VFO_MENUS 3
static const struct sMenuItem vfoMenu[NUM_VFO_MENUS] =
{
    { "",               NULL },
    { "VFO Band",       menuVFOBand },
    { "VFO Mode",       menuVFOMode },
};

#define NUM_TEST_MENUS 10
static const struct sMenuItem testMenu[NUM_TEST_MENUS] =
{
    { "",               NULL },
    { "Break in",       menuBreakIn },
    { "Test RX Mute",   menuTestRXMute },
    { "Sidetone",       menuSidetone },
    { "RX Clock",       menuRXClock },
    { "Unmute delay",   menuUnmuteDelay },
    { "Mute delay",     menuMuteDelay },
    { "TX delay",       menuTXDelay },
    { "TX Clock",       menuTXClock },
    { "TX Out",         menuTXOut },
};

#define NUM_CONFIG_MENUS 3
static const struct sMenuItem configMenu[NUM_CONFIG_MENUS] =
{
    { "",               NULL },
    { "Xtal Frequency", menuXtalFreq },
    { "Keyer Mode",     menuKeyerMode },
};

enum eMenuTopLevel
{
    VFO_MENU,
    TEST_MENU,
    CONFIG_MENU,
    NUM_MENUS
};

static const struct
{
    char                    *text;
    const struct sMenuItem  *subMenu;
    uint8_t                 numItems;
}
menu[NUM_MENUS] =
{
    { "VFO",    vfoMenu,    NUM_VFO_MENUS },
    { "Test",   testMenu,   NUM_TEST_MENUS },
    { "Config", configMenu, NUM_CONFIG_MENUS },
};

static uint8_t currentMenu;
static uint8_t currentSubMenu;

// Set to true when we are in a menu item rather than in the top level
static bool bInMenuItem;

// Set to true if we enter a menu item
// Allows us to only enter wpm state if we have not been in a menu
static bool bEnteredMenuItem;

// Set to true if entered the VFO menu quickly so that we
// can leave quickly
static bool bInQuickVFOMenu;

// Text for the quick menu line
// In split mode cannot enter RIT or XIT so show them in lower case
// Also show we are in split mode
#define QUICK_MENU_TEXT         "A/B A=B R X SPLT"
#define QUICK_MENU_SPLIT_TEXT   "A/B A=B r x splt"

// Quick menu array
struct sQuickMenuItem
{
    uint8_t pos;    // Position on the line - needs to match the above text
    void (*func)(); // Function to call when item is selected
};

static void quickMenuSwap();
static void quickMenuEqual();
static void quickMenuRIT();
static void quickMenuXIT();
static void quickMenuSplit();

#define NUM_QUICK_MENUS 5
static const struct sQuickMenuItem quickMenu[NUM_QUICK_MENUS] =
{
    { 0,  quickMenuSwap },
    { 4,  quickMenuEqual },
    { 8,  quickMenuRIT },
    { 10, quickMenuXIT },
    { 12, quickMenuSplit },
};

// Current quick menu item
static uint8_t quickMenuItem;

// Current user interface mode we are in
static enum eCurrentMode
{
    modeVFO, // Default tuning mode
    modeFastVFO,
    modeMenu,
    modeWpm,
    modeQuickMenu,
} currentMode = modeVFO;
#endif

// Band frequencies
#ifdef SOTA2
#define NUM_BANDS 2
#else
#define NUM_BANDS 13
#endif

static const struct  
{
    char     *bandName;     // Text for the menu
    uint32_t  minFreq;      // Min band frequency
    uint32_t  maxFreq;      // Max band frequency
    uint32_t  defaultFreq;  // Where to start on this band e.g. QRP calling
#ifdef SOTA2
    uint32_t  leftFreq;     // Below this frequency light the left LED
    uint32_t  rightFreq;    // Above this frequency light the right LED
#endif
    bool      bTXEnabled;   // True if TX enabled on this band
    uint8_t   relayState;   // What state to put the relays in on this band
#ifndef SOTA2
    bool      bQuickVFOMenu;// True if this band appears in the quick VFO menu
#endif
}
band[NUM_BANDS] =
{
#ifdef SOTA2
    { "40m",      7000000,  7199999,  7030000,   7020000,  7040000, TX_ENABLED_40M,  RELAY_STATE_40M },
    { "20m",     14000000, 14349999, 14060000,  14050000, 14070000, TX_ENABLED_20M,  RELAY_STATE_20M },
#else
    { "160m",     1810000,  1999999,  1836000, TX_ENABLED_160M, RELAY_STATE_160M, QUICK_VFO_160M },
    { "80m",      3500000,  3799999,  3560000, TX_ENABLED_80M,  RELAY_STATE_80M,  QUICK_VFO_80M },
    { "RWM 4996", 4996000,  4996000,  4996000, false,           RELAY_STATE_60M,  false },
    { "60m UK",   5258500,  5263999,  5262000, TX_ENABLED_60M,  RELAY_STATE_60M,  QUICK_VFO_60M },
    { "60m EU",   5354000,  5357999,  5355000, TX_ENABLED_60M,  RELAY_STATE_60M,  QUICK_VFO_60M },
    { "40m",      7000000,  7199999,  7030000, TX_ENABLED_40M,  RELAY_STATE_40M,  QUICK_VFO_40M },
    { "RWM 9996", 9996000,  9996000,  9996000, false,           RELAY_STATE_30M,  false },
    { "30m",     10100000, 10150000, 10116000, TX_ENABLED_30M,  RELAY_STATE_30M,  QUICK_VFO_30M },
    { "20m",     14000000, 14349999, 14060000, TX_ENABLED_20M,  RELAY_STATE_20M,  QUICK_VFO_20M },
    { "17m",     18068000, 18167999, 18086000, TX_ENABLED_17M,  RELAY_STATE_17M,  QUICK_VFO_17M },
    { "15m",     21000000, 21449999, 21060000, TX_ENABLED_15M,  RELAY_STATE_15M,  QUICK_VFO_15M },
    { "12m",     24890000, 24989999, 24906000, TX_ENABLED_12M,  RELAY_STATE_12M,  QUICK_VFO_12M },
    { "10m",     28000000, 29699999, 28060000, TX_ENABLED_10M,  RELAY_STATE_10M,  QUICK_VFO_10M },
#endif
};

// Current band - initialised from NVRAM
static uint8_t currentBand;

// Current relay state - always set from the frequency
static uint8_t currentRelay;

#ifndef SOTA2
// Is the VFO on the first or second frequency line?
static bool bVFOFirstLine = true;


// For the xtal frequency the current digit position that is changing
static uint8_t xtalFreqPos;

// True if asking whether to save the xtal frequency setting
static bool bAskToSaveXtalFreq = false;

#endif
	
// Set to true if break in is enabled
static bool bBreakIn = true;

// Set to true if the oscillator is successfully initialised over I2C
static bool bOscInit;

// VFO modes
enum eVFOMode
{
    vfoSimplex,
    vfoRIT,
    vfoXIT,
    vfoNumModes // Num of VFO modes. Must be the last entry.
};

#ifndef SOTA2
// The cursor position along with its corresponding frequency change
struct sCursorPos
{
    uint8_t x, y;
    uint32_t freqChange;
};

// Mark the end of the cursor transitions
#define CURSOR_TRANSITION_END 0xFF

// The cursor transitions for the VFO
#define NUM_CURSOR_TRANSITIONS 6
static const struct sCursorPos vfoCursorTransition[NUM_CURSOR_TRANSITIONS] =
{
    { 9, 1, 10 },
    { 8, 1, 100 },
    { 7, 1, 250 },
    { 6, 1, 1000 },
    { 5, 1, 10000 },
    { CURSOR_TRANSITION_END, CURSOR_TRANSITION_END, CURSOR_TRANSITION_END }
};

// Index for the current cursor position
static uint8_t cursorIndex;

// In fast mode, if the dial is spun the rate speeds up
#define VFO_SPEED_UP_DIFF  150  // If dial clicks are no more than this ms apart then speed up
#define VFO_SPEED_UP_FACTOR 10  // Multiply the rate by this
#endif

static void rotaryVFO( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight );

// Maintain transmit and receive frequencies for two VFOs
// The current VFO
static uint8_t currentVFO;

// Macro to give the other VFO
#define OTHER_VFO ((currentVFO)^1)

// Macro to give the letter for the VFO i.e. A or B
#define VFO_LETTER(vfo) (((vfo)==VFO_A)?'A':'B')

// State of each vfo
static struct sVFOState 
{
    uint32_t        freq;       // Frequency
    int16_t         offset;     // Offset when in RIT or XIT
    enum eVFOMode   mode;       // Simplex, RIT or XIT
} vfoState[NUM_VFOS];

// True if in split mode (RX on current VFO, TX on other VFO)
static bool bVFOSplit;

// Set to true when testing RX mute function
static bool bTestRXMute;

// Set to true when RX clock enabled
static bool bRXClockEnabled = true;

// Set to true when TX clock enabled
static bool bTXClockEnabled = true;

// Set to true when morse output enabled
static bool bTXOutEnabled = true;

// Set to true when sidetone enabled
static bool bSidetone = true;

// Delay before muting and unmuting
static uint8_t muteDelay = 5;
static uint8_t unmuteDelay = 5;
static uint8_t txDelay = 10;

// Set to true when transmitting
static bool bTransmitting = false;

#ifndef SOTA2
static void update_display();
#endif

// Works out the current RX frequency from the VFO settings
static uint32_t getRXFreq()
{
    uint32_t freq;

    // Start with the current VFO's base frequency
    freq = vfoState[currentVFO].freq;

    // If in RIT mode need to add the offset
    if( vfoState[currentVFO].mode == vfoRIT )
    {
        freq += vfoState[currentVFO].offset;
    }

    return freq;
}

// Works out the current TX frequency from the VFO settings
static uint32_t getTXFreq()
{
    uint32_t freq;
    uint8_t vfo;

    // If in split mode then the TX frequency comes from the other VFO,
    // else it's the current VFO
    if( bVFOSplit )
    {
        vfo = OTHER_VFO;
    }
    else
    {
        vfo = currentVFO;
    }

    // Start with the VFO's base frequency
    freq = vfoState[vfo].freq;

    // If in XIT mode need to add the offset
    if( vfoState[vfo].mode == vfoXIT )
    {
        freq += vfoState[vfo].offset;
    }

    return freq;
}

// Returns true if the TX is enabled at the current transmit frequency
static bool txEnabled()
{
    // Assume TX is not enabled
    bool bEnabled = false;

    // If the TX frequency is in the current band then get its TX enabled status
    if( (getTXFreq() >= band[currentBand].minFreq) && (getTXFreq() <= band[currentBand].maxFreq) )
    {
        bEnabled = band[currentBand].bTXEnabled;
    }

    return bEnabled;
}

// See if this is a new band and if so set the new band
void setBandFromFrequency( uint32_t freq )
{
    // Is the frequency outside the current band limits?
    if( (freq < band[currentBand].minFreq) || (freq > band[currentBand].maxFreq) )
    {
        // Yes it is
        // See what band it is in
        for( uint8_t b = 0 ; b < NUM_BANDS ; b++ )
        {
            if( freq <= band[b].maxFreq )
            {
                // Whether or not we are in band this must be the relay state we need
                // for the correct LPF
                currentRelay = band[b].relayState;

                currentBand = b;

                // Store the band in the NVRAM
                nvramWriteBand( b );

                // We can stop looking whether out of band or not
                break;
            }
        }
    }
}

// Sets the relay for the current frequency
static void setRelay()
{
#if NUM_RELAYS == 1
    // If there is only one relay then it is either on or off rather
    // than a relay per band
    ioWriteBandRelay( 0, currentRelay );
#else
    // Set each relay to off except for the current relay
    for( int i = 0 ; i < NUM_RELAYS ; i++ )
    {
        ioWriteBandRelay( i, currentRelay == i );
    }
#endif
}

// Enable/disable the RX clock outputs
static void enableRXClock( bool bEnable )
{
    oscClockEnable( RX_CLOCK_A, bEnable );
    oscClockEnable( RX_CLOCK_B, bEnable );
}

// Enable/disable the TX clock output
static void enableTXClock( bool bEnable )
{
    oscClockEnable( TX_CLOCK, bEnable );
}

// Turns on/off the TX clock and PA
// If break-in off disables this
static void Transmit( bool bTX )
{
    if( bBreakIn )
    {
        if( bTX )
        {
            // Turn on the TX clock
            if( bTXClockEnabled )
            {
                enableTXClock( true );
                delay(txDelay);
            }

            // Set the morse output high
            if( bTXOutEnabled )
            {
                ioWriteMorseOutputHigh();
            }
        }
        else
        {
            // Delay the same as before transmit so that the dot
            // or dash is not truncated
            delay(txDelay);

            // Set the morse output low
            ioWriteMorseOutputLow();
            delay(txDelay);

            // Turn off the TX clock
            enableTXClock( false );
        }
        bTransmitting = bTX;
    }
}

static void muteRX( bool bMute )
{
    if( bMute )
    {
        ioWriteRXEnableLow();
    }
    else
    {
        ioWriteRXEnableHigh();
    }
}

static void sidetoneOn( bool bOn )
{
    if( bSidetone )
    {
        if( bOn )
        {
            ioWriteSidetoneOn();
        }
        else
        {
            ioWriteSidetoneOff();
        }
    }
}

// Handle key up and down - mute RX, transmit, sidetone etc.
void keyDown( bool bDown )
{
    if( bDown )
    {
        // Key down only if TX is enabled on the current
        // TX frequency
        if( txEnabled() )
        {
            if( !bTestRXMute )
            {
                // Mute RX first
                muteRX( true );
                delay(muteDelay);
            }

            if( bRXClockEnabled )
            {
                // Turn off the RX clock
                enableRXClock( false );
            }

            // Turn on the TX oscillator and PA
            Transmit( true );

            // Turn on sidetone
            sidetoneOn( true );
        }
    }
    else
    {
        // Delay the same as before key down so that dot or dash
        // is not truncated
        delay(muteDelay);

        // Turn off the PA and TX oscillator
        Transmit( false );

        // Turn off sidetone
        sidetoneOn( false );

        if( bRXClockEnabled )
        {
            // Turn on the RX clock
            enableRXClock( true );
        }

        if( !bTestRXMute )
        {
            // Unmute the RX last
            delay(unmuteDelay);
            muteRX( false );
        }
    }
}

// Display the morse character if not in the menu
void displayMorse( char *text )
{
/*
    if( currentMode != modeMenu )
    {
        displayText( MORSE_LINE, text, false );
    }
*/
}

#ifdef SOTA2

// For the SOTA transceiver we have 3 LEDs instead of an LCD.
// The default frequency is the centre frequency (e.g. 14060)
// and we have left and right frequencies (e.g. 14050 and 14070)
// Centre LED lit between the left and right frequencies
// Left LED lit below the centre
// Right LED lit above the centre
// All LEDs light out of band
static void update_display()
{
    uint32_t freq = getRXFreq();

    // Check we are in band
    if( (freq >= band[currentBand].minFreq) &&
        (freq <= band[currentBand].maxFreq) )
    {
        // Centre LED is lit if between the left and right frequencies
        ioWriteCentreLED( (freq >= band[currentBand].leftFreq) &&
                          (freq <= band[currentBand].rightFreq) );

        // Left LED is lit if below the centre frequency
        ioWriteLeftLED( freq < band[currentBand].defaultFreq);

        // Right LED is lit if above the centre frequency
        ioWriteRightLED( freq > band[currentBand].defaultFreq );
    }
    else
    {
        // Out of band so light all the LEDs
        ioWriteLeftLED( true );
        ioWriteCentreLED( true );
        ioWriteRightLED( true );
    }
}

#else

// Set the correct cursor for the VFO mode
static void update_cursor()
{
    // Default to the top frequency line
    uint8_t line = FREQ_LINE;

    // Only update the cursor if in VFO mode
    if( currentMode == modeVFO || currentMode == modeFastVFO )
    {
        // If split, RIT or XIT mode may need to put the cursor on the other line
        if( bVFOSplit || vfoState[currentVFO].mode != vfoSimplex )
        {
            // Display the cursor on the correct line
            if( !bVFOFirstLine )
            {
                line = FREQ_LINE + 1;
            }
        }

        displayCursor( vfoCursorTransition[cursorIndex].x, line, cursorUnderline );
    }
}

// Update the display with the frequency and morse wpm
static void update_display()
{
    char wpmText[TEXT_BUF_LEN];
    char freqText[TEXT_BUF_LEN*2];
    
    // Frequency for the first and second lines
    uint32_t freq1 = 0;
    uint32_t freq2 = 0;
    
    // Character at the start of each line e.g. A, B, X, R, +, -
    char cLine1A = ' ';
    char cLine1B = ' ';
    char cLine2A = ' ';
    char cLine2B = ' ';

    // The dot to separate kHz from hundreds
    // Changes to 'x' if TX not enabled for the current
    // TX frequency
    char cDot = txEnabled() ? '.' : 'x';
        
    // Get the current wpm.
    uint8_t wpm = morseGetWpm();

    // Create the text for the morse speed
    if( wpm > 0 )
    {
        sprintf( wpmText, "%2dwpm", wpm);
    }
    else
    {
        // A morse wpm of 0 means straight key mode
        // May be in tune mode (continuous transmit until dot pressed)
        if( morseInTuneMode() )
        {
            sprintf( wpmText, "Tune " );
        }
        else
        {
            sprintf( wpmText, "SKey " );
        }
    }

    // First line begins with a letter to tell us which VFO (A or B) or
    // if the oscillator is not OK (N)
    if( bOscInit )
    {
        cLine1A = VFO_LETTER(currentVFO);
    }
    else
    {
        cLine1A = 'N';
    }

    if( bVFOSplit )
    {
        // In split mode the second line has the transmit frequency
        freq2 = getTXFreq();
        cLine2A = VFO_LETTER(OTHER_VFO);
    }
        
    // If in RIT or XIT display the appropriate letter
    if( vfoState[currentVFO].mode == vfoRIT || vfoState[currentVFO].mode == vfoXIT )
    {
        if( vfoState[currentVFO].mode == vfoRIT )
        {
            cLine1B = 'r';
            cLine2A = 'R';
        }
        else
        {
            cLine1B = 'x';
            cLine2A = 'X';
        }

        // The second line has the offset
        // If the offset is negative then display it with a minus sign
        if( vfoState[currentVFO].offset < 0 )
        {
            freq2 = -vfoState[currentVFO].offset;
            cLine2B = '-';
        }
        else
        {
            // Positive offset
            freq2 = vfoState[currentVFO].offset;
            cLine2B = '+';
        }
    }

    // Line 1 usually has the RX frequency but during transmit may show the TX frequency
    if( !bTransmitting || bVFOSplit || (vfoState[currentVFO].mode == vfoSimplex) )
    {
        // If in receive mode, or split, or simplex , then first line is the receive frequency
        freq1 = getRXFreq();
    }
    else
    {
        // If transmitting in RIT or XIT mode, first line is the transmit frequency
        freq1 = getTXFreq();
    }

    // Line 1 has the frequency as determined above plus the morse WPM
    sprintf( freqText, "%c%c%5lu%c%02lu %s", cLine1A, cLine1B, freq1/1000, cDot, (freq1%1000)/10, wpmText);
    displayText( FREQ_LINE, freqText, true );
    
    // All modes other than simplex have a second line
    if( bVFOSplit || (vfoState[currentVFO].mode != vfoSimplex) )
    {
        // Second line is split so frequency not overwritten
        displaySplitLine( 10, FREQ_LINE + 1 );
        sprintf( freqText, "%c%c%5lu%c%02lu", cLine2A, cLine2B, freq2/1000, cDot, (freq2%1000)/10);
        displayText( FREQ_LINE + 1, freqText, true );
    }
    else
    {
        // Second line is not split and needs to be cleared if not in the menu or quick menu
        if( (currentMode != modeQuickMenu) && (currentMode != modeMenu) )
        {
            displaySplitLine( 0, FREQ_LINE + 1 );
            displayText( FREQ_LINE + 1, "", true );
        }
    }
}
#endif

// Set the RX frequency
static void setRXFrequency( uint32_t freq )
{
    // The RX oscillator has to be offset for the CW tone to be audible
    // Decide if we are using CW normal or reverse
    bool bCWReverse = nvramReadCWReverse();
    uint32_t oscFreq;

    if( bCWReverse )
    {
        // Reverse mode is LSB so need the LO above the RX frequency
        oscFreq = freq + RX_OFFSET;
    }
    else
    {
        // Normal mode is USB so need the LO below the RX frequency
        oscFreq = freq - RX_OFFSET;
    }

    // Set the oscillator frequency. This will set the correct quadrature
    // phase shift.
    oscSetFrequency( RX_CLOCK_A, oscFreq, 0 );
    oscSetFrequency( RX_CLOCK_B, oscFreq, bCWReverse ? 1 : -1 );

    // Set the relay
    setRelay();
}

// Set the TX and RX frequencies
static void setFrequencies()
{
    // See if this is a new band and if so set the new band
    setBandFromFrequency( getRXFreq() );

    // Set RX and TX frequencies
    setRXFrequency( getRXFreq() );
    oscSetFrequency( TX_CLOCK, getTXFreq(), 0 );

    // Ensure the display and cursor reflect this
    update_display();
#ifndef SOTA2
    update_cursor();
#endif
}

// Set the band - sets the frequencies and memories to the new band's default
static void setBand( int newBand )
{
    // Set the default frequency for both VFOs
    // Go into simplex mode
    vfoState[VFO_A].freq = band[newBand].defaultFreq;
    vfoState[VFO_A].offset = 0;
    vfoState[VFO_A].mode = vfoSimplex;
    vfoState[VFO_B].freq = band[newBand].defaultFreq;
    vfoState[VFO_B].offset = 0;
    vfoState[VFO_B].mode = vfoSimplex;

    // Turn off split mode
    bVFOSplit = false;

    // Set the new TX and RX frequencies
    setFrequencies();
}

#ifndef SOTA2
// Display the menu text for the current menu or sub menu
static void menuDisplayText()
{
    char text[TEXT_BUF_LEN];

    // Turn off the split line for the menu
    displaySplitLine( 0, MENU_LINE );
    
    // Display either the menu or the sub-menu text
    if( currentSubMenu == 0 )
    {
        // Not in a sub-menu
        sprintf( text, "%c %s", 'A' + currentMenu, menu[currentMenu].text );
    }
    else
    {
        // In a sub-menu
        sprintf( text, "%c.%d %s", 'A' + currentMenu, currentSubMenu, menu[currentMenu].subMenu[currentSubMenu].text );
    }
    
    displayText( MENU_LINE, text, true );
    
    // Turn off the cursor
    displayCursor( 0, 0, cursorOff );
}

// Enter the wpm setting mode
static void enterWpm()
{
    // Make the cursor blink on the wpm
    displayCursor( WPM_COL, WPM_LINE, cursorBlink );

    currentMode = modeWpm;
}

// Enter the menu
static void enterMenu()
{
    // In the menu but not yet in a menu item
    currentMenu = currentSubMenu = 0;
    currentMode = modeMenu;
    bInMenuItem = false;

    // Display the current menu text
    menuDisplayText();
}

// Quick way to the VFO menu
static void enterVFOBandMenu()
{
    currentMenu = VFO_MENU;
    currentSubMenu = 1;
    currentMode = modeMenu;
    bInMenuItem = true;

    // We are entering the menu quickly so we can get out quickly
    bInQuickVFOMenu = true;

    // Display the current menu text
    menuVFOBand(false, false, false, false, false, false, false, false);

    // Turn off the cursor
    displayCursor( 0, 0, cursorOff );
}

// Go back to VFO mode
static void enterVFOMode()
{
    // Go to VFO mode
    currentMode = modeVFO;

    // Next time we enter the menu start not in a menu item
    bEnteredMenuItem = false;

    // Display the cursor in the right place
    update_cursor();

    // Clear the second line as no longer in the menu
    displaySplitLine( 0, FREQ_LINE + 1 );
    displayText( FREQ_LINE+1, "", true);

    // Update the display with the correct split for the current VFO mode
    update_display();
}

// Swap the VFOs - called either from the quick menu or CAT control
void vfoSwap()
{
    // Swap the VFOs
    currentVFO = OTHER_VFO;

    // Update the frequencies and display
    setFrequencies();
    update_display();
}

static void quickMenuSwap()
{
    vfoSwap();
}

// Set the other VFO to the current VFO - called either from the quick menu or CAT control
void vfoEqual()
{
    vfoState[OTHER_VFO] = vfoState[currentVFO];

    // If in split mode need to ensure the transmit frequency is updated
    if( bVFOSplit )
    {
        setFrequencies();
    }
}

// Set the other VFO to the current VFO
static void quickMenuEqual()
{
    vfoEqual();
}

// Set RIT on or off - called from quick menu or CAT control
void setCurrentVFORIT( bool bRIT )
{
    // Ignore if in split mode
    if( !bVFOSplit )
    {
        if( bRIT )
        {
            vfoState[currentVFO].mode = vfoRIT;

            // Always on the second line in RIT mode
            bVFOFirstLine = false;
        }
        else
        {
            vfoState[currentVFO].mode = vfoSimplex;
        }

        // Update the frequencies and display
        setFrequencies();
        update_cursor();
    }
}

// Quick menu item RIT selected
static void quickMenuRIT()
{
    switch( vfoState[currentVFO].mode )
    {
        // If in RIT then back to simplex
        case vfoRIT:
            setCurrentVFORIT( false );
            break;
        // If in simplex or XIT then go to RIT
        case vfoSimplex:
        case vfoXIT:
        default:
            setCurrentVFORIT( true );
            break;
    }

    enterVFOMode();
}

// Set XIT on or off - called from quick menu or CAT control
void setCurrentVFOXIT( bool bXIT )
{
    // Ignore if in split mode
    if( !bVFOSplit )
    {
        if( bXIT )
        {
            vfoState[currentVFO].mode = vfoXIT;

            // Always on the second line in XIT mode
            bVFOFirstLine = false;
        }
        else
        {
            vfoState[currentVFO].mode = vfoSimplex;
        }

        // Update the frequencies and display
        setFrequencies();
        update_cursor();
    }
}

// Quick menu item XIT selected
static void quickMenuXIT()
{
    switch( vfoState[currentVFO].mode )
    {
        // If in XIT then back to simplex
        case vfoXIT:
            setCurrentVFOXIT( false );
            break;

        // If in simplex or RIT then go to XIT
        case vfoSimplex:
        case vfoRIT:
        default:
            setCurrentVFOXIT( true );
            break;
    }

    enterVFOMode();
}

// Set the VFO split state - for CAT control or the quick menu
void setVFOSplit( bool bSplit )
{
    // Ignore if no change
    if( bSplit != bVFOSplit )
    {
        // Moving into split
        if( bSplit )
        {
            // Before moving into split mode, work out the current
            // TX and RX frequencies.
            uint32_t txFreq = getTXFreq();
            uint32_t rxFreq = getRXFreq();

            // Now ensure RIT and XIT are off with zero offsets
            vfoState[currentVFO].mode = vfoSimplex;
            vfoState[currentVFO].offset = 0;
            vfoState[OTHER_VFO].mode = vfoSimplex;
            vfoState[OTHER_VFO].offset = 0;

            // Set the current VFO to the RX frequency and the other
            // VFO to the TX frequency
            vfoState[currentVFO].freq = rxFreq;
            vfoState[OTHER_VFO].freq = txFreq;
        }

        bVFOSplit = bSplit;

        // Update the frequencies and display
        setFrequencies();
    }
}

// Quick menu item Split selected
static void quickMenuSplit()
{
    // Toggle split state
    setVFOSplit( !bVFOSplit );
    enterVFOMode();
}

// Display the quick menu text
static void quickMenuDisplayText()
{
    // Turn off the split line for the menu
    displaySplitLine( 0, MENU_LINE );

    // Display the quick menu
    // Slightly different text in split mode
    displayText( MENU_LINE, (bVFOSplit ? QUICK_MENU_SPLIT_TEXT : QUICK_MENU_TEXT), true );
    
    // Make the cursor blink on the current item
    displayCursor( quickMenu[quickMenuItem].pos, MENU_LINE, cursorBlink );
}

// Enter the quick menu
static void enterQuickMenu()
{
    currentMode = modeQuickMenu;

    // Start with the first item
    quickMenuItem = 0;

    // Display the current menu text
    quickMenuDisplayText();
}

// Handle the rotary control while in the menu
static void rotaryMenu( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // If in a menu item then pass control to its function
    if( bInMenuItem )
    {
        // We'll let the menu function have first go at dealing with
        // any presses etc. Only if it hasn't used it will we do
        // anything
        if( !menu[currentMenu].subMenu[currentSubMenu].func( bCW, bCCW, bShortPress, bLongPress, bShortPressLeft, bLongPressLeft, bShortPressRight, bLongPressRight ) )
        {
            // A long press takes us out of the menu item
            if( bLongPress )
            {
                // Now out of the menu item
                bInMenuItem = false;

                // Display the current menu text
                menuDisplayText();
            }
        }
    }
    else
    {
        // May be in the top level menu or a sub menu
        if( currentSubMenu == 0 )
        {
            // In the top level menu
            if( bShortPressRight )
            {
                if( currentMenu == (NUM_MENUS-1))
                {
                    currentMenu = 0;
                }
                else
                {
                    currentMenu++;
                }

                // Display the new menu item
                menuDisplayText();
            }
            else if( bShortPressLeft )
            {
                if( currentMenu == 0)
                {
                    currentMenu = (NUM_MENUS-1);
                }
                else
                {
                    currentMenu--;
                }

                // Display the new menu item
                menuDisplayText();
            }
            else if( bShortPress )
            {
                // A short press takes us into the sub menu
                currentSubMenu = 1;

                // Display the new menu item
                menuDisplayText();
            }
            else if( bLongPress )
            {
                enterVFOMode();
            }
        }
        else
        {
            // In a sub menu
            if( bShortPressRight )
            {
                if( currentSubMenu == (menu[currentMenu].numItems-1))
                {
                    currentSubMenu = 1;
                }
                else
                {
                    currentSubMenu++;
                }

                // Display the new menu item
                menuDisplayText();
            }
            else if( bShortPressLeft )
            {
                if( currentSubMenu == 1)
                {
                    currentSubMenu = (menu[currentMenu].numItems-1);
                }
                else
                {
                    currentSubMenu--;
                }

                // Display the new menu item
                menuDisplayText();
            }
            else if( bShortPress )
            {
                // Now in the menu item
                bInMenuItem = true;
                bEnteredMenuItem = true;

                // A short press on a menu item calls its function
                menu[currentMenu].subMenu[currentSubMenu].func( false, false, false, false, false, false, false, false );
            }
            else if( bLongPress )
            {
                // A long press take us out of the sub menu
                currentSubMenu = 0;

                // Display the new menu item
                menuDisplayText();
            }
        }
    }
}

// Return to simplex VFO mode
static void enterSimplex()
{
    setCurrentVFORIT( false );
    setCurrentVFOXIT( false );
    setVFOSplit( false );
    enterVFOMode();
}

// Handle the rotary control while in the quick menu
static void rotaryQuickMenu( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Rotary movement continues to operate the VFO
    if( bCW || bCCW )
    {
        rotaryVFO(bCW, bCCW, bShortPress, bLongPress, bShortPressLeft, bLongPressLeft, bShortPressRight, bLongPressRight);
    }
    else if( bShortPressRight )
    {
        if( quickMenuItem == (NUM_QUICK_MENUS-1))
        {
            quickMenuItem = 0;
        }
        else
        {
            quickMenuItem++;
        }

        // Display the new menu item
        quickMenuDisplayText();
    }
    else if( bShortPressLeft )
    {
        if( quickMenuItem == 0)
        {
            quickMenuItem = (NUM_QUICK_MENUS-1);
        }
        else
        {
            quickMenuItem--;
        }

        // Display the new menu item
        quickMenuDisplayText();
    }
    else if( bShortPress )
    {
        // A short press calls the function for this item
        quickMenu[quickMenuItem].func();
    }
    else if( bLongPress )
    {
        // A long press take us out of the menu
        enterVFOMode();
    }
    else if( bLongPressLeft )
    {
        // A long left press puts us back to simplex VFO mode
        enterSimplex();
    }
    else if( bLongPressRight )
    {
        // A long right press puts us into WPM mode
        enterWpm();
    }
}

// Returns the next band. Allows for skipping bands that do not appear in
// the quick VFO menu.
//
// oldBand      The previous band
// direction    +1 to go up a band or -1 to go down a band
// bAllBands    true if all bands in the table are allowed
//              false means we will skip bands that are not to appear in the
//              quick VFO menu
static uint8_t nextBand( uint8_t oldBand, int8_t direction, bool bAllBands )
{
    // The new band starts with the old band - signed as may try to go back from 0
    int8_t newBand = oldBand;

    // We keep looping until we find a band we want to appear
    for(;;)
    {
        newBand += direction;

        if( newBand >= NUM_BANDS )
        {
            newBand = 0;
        }
        else if( newBand < 0 )
        {
            newBand = NUM_BANDS - 1;
        }

        // If all bands are valid then can stop looping
        // otherwise only stop if the band is meant to appear
        if( bAllBands || band[newBand].bQuickVFOMenu )
        {
            break;
        }
    }

    return newBand;
}

// Handle the menu for the VFO band
static bool menuVFOBand( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    // The new band - need to maintain this between calls
    static int newBand;
    
    // If just entered the menu note the current band
    if( !bShortPressRight && !bShortPressLeft && !bShortPress &&!bLongPress )
    {
        newBand = currentBand;
    }

    // Right and left buttons change band
    if( bShortPressRight )
    {
        newBand = nextBand( newBand, +1, !bInQuickVFOMenu );
        bUsed = true;
    }
    else if( bShortPressLeft )
    {
        newBand = nextBand( newBand, -1, !bInQuickVFOMenu );
        bUsed = true;
    }

    // Display the current band
    char buf[TEXT_BUF_LEN];
    sprintf( buf, "Band: %s", band[newBand].bandName);
    displayText( MENU_LINE, buf, true );

    // Short press sets the new band
    if( bShortPress )
    {
        // Nothing to do unless the band has changed
        if( newBand != currentBand )
        {
            // Set the new band
            setBand( newBand );

            // Store the band in the NVRAM
            nvramWriteBand( newBand );

            // Leave the menu and go back to VFO mode
            enterVFOMode();

            // Left the menu
            bInQuickVFOMenu = false;
        }
        
        bUsed = true;
    }

    // If we entered the menu quickly then a long press takes us out
    // and back into VFO mode
    if( bLongPress && bInQuickVFOMenu )
    {
        bInQuickVFOMenu = false;
        bInMenuItem = false;
        enterVFOMode();
        bUsed = true;
    }
    
    return bUsed;
}

// Set the CW reverse state
void setCWReverse( bool bCWReverse )
{
    // Store in the NVRAM
    nvramWriteCWReverse( bCWReverse );

    // Action the change in sideband
    setFrequencies();
}

static bool menuVFOMode( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Get the CW mode from NVRAM
    bool bCWReverse = nvramReadCWReverse();

    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    // Left or right toggles
    if( bShortPressLeft || bShortPressRight )
    {
        // Toggle the CW mode
        bCWReverse = !bCWReverse;

        // Set the new mode
        setCWReverse( bCWReverse );

        bUsed = true;
    }

    if( bCWReverse )
    {
        displayText( MENU_LINE, "VFO CW Reverse", true );
    }
    else
    {
        displayText( MENU_LINE, "VFO CW Normal", true );
    }
    
    return bUsed;
}

static bool menuBreakIn( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    // Left or right toggles
    if( bShortPressLeft || bShortPressRight )
    {
        bBreakIn = !bBreakIn;
        bUsed = true;
    }

    if( bBreakIn )
    {
        displayText( MENU_LINE, "Break in: On", true );
    }
    else
    {
        displayText( MENU_LINE, "Break in: Off", true );
    }
    
    return bUsed;
}

static bool menuSidetone( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    // Left or right toggles
    if( bShortPressLeft || bShortPressRight )
    {
        bSidetone = !bSidetone;
        bUsed = true;
    }

    if( bSidetone )
    {
        displayText( MENU_LINE, "Sidetone: Enabled", true );
    }
    else
    {
        displayText( MENU_LINE, "Sidetone: Disabled", true );
    }
    
    return bUsed;
}

static bool menuTestRXMute( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    // Left or right toggles
    if( bShortPressLeft || bShortPressRight )
    {
        bTestRXMute = !bTestRXMute;
        bUsed = true;
    }

    if( bTestRXMute )
    {
        muteRX( true );
        displayText( MENU_LINE, "Test RX Mute: On", true );
    }
    else
    {
        muteRX( false );
        displayText( MENU_LINE, "Test RX Mute: Off", true );
    }
    
    return bUsed;
}


static bool menuRXClock( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    // Left or right toggles
    if( bShortPressLeft || bShortPressRight )
    {
        bRXClockEnabled = !bRXClockEnabled;
        bUsed = true;
    }

    if( bRXClockEnabled )
    {
        enableRXClock( true );
        displayText( MENU_LINE, "RX Clock: Enabled", true );
    }
    else
    {
        enableRXClock( false );
        displayText( MENU_LINE, "RX Clock: Disabled", true );
    }
    
    return bUsed;
}

static bool menuTXDelay( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    if( bCW )
    {
        if( txDelay < 50 )
        {
            txDelay++;
        }
        else if( txDelay < 500 )
        {
            txDelay += 50;
        }
        bUsed = true;
    }
    else if( bCCW )
    {
        if( txDelay > 50 )
        {
            txDelay -= 50;
        }
        else if( txDelay > 0 )
        {
            txDelay--;
        }
        bUsed = true;
    }
    else if( bShortPress )
    {
        txDelay = 10;
        bUsed = true;
    }
    char buf[TEXT_BUF_LEN];
    sprintf( buf, "TX delay: %d", txDelay);
    displayText( MENU_LINE, buf, true );
    
    return bUsed;
}

static bool menuTXClock( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    // Left or right toggles
    if( bShortPressLeft || bShortPressRight )
    {
        bTXClockEnabled = !bTXClockEnabled;
        bUsed = true;
    }

    if( bTXClockEnabled )
    {
        displayText( MENU_LINE, "TX Clock: Enabled", true );
    }
    else
    {
        displayText( MENU_LINE, "TX Clock: Disabled", true );
    }
    
    return bUsed;
}

static bool menuTXOut( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    // Left or right toggles
    if( bShortPressLeft || bShortPressRight )
    {
        bTXOutEnabled = !bTXOutEnabled;
        bUsed = true;
    }

    if( bTXOutEnabled )
    {
        displayText( MENU_LINE, "TX Out: Enabled", true );
    }
    else
    {
        displayText( MENU_LINE, "TX Out: Disabled", true );
    }
    
    return bUsed;
}

static bool menuUnmuteDelay( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    if( bCW )
    {
        if( unmuteDelay < 50 )
        {
            unmuteDelay++;
        }
        else if( unmuteDelay < 500 )
        {
            unmuteDelay += 50;
        }
        bUsed = true;
    }
    else if( bCCW )
    {
        if( unmuteDelay > 50 )
        {
            unmuteDelay -= 50;
        }
        else if( unmuteDelay > 0 )
        {
            unmuteDelay--;
        }
        bUsed = true;
    }
    else if( bShortPress )
    {
        unmuteDelay = 5;
        bUsed = true;
    }
    char buf[TEXT_BUF_LEN];
    sprintf( buf, "Unmute dly: %d", unmuteDelay);
    displayText( MENU_LINE, buf, true );
    
    return bUsed;
}


static bool menuMuteDelay( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    if( bCW )
    {
        if( muteDelay < 50 )
        {
            muteDelay++;
        }
        else if( muteDelay < 500 )
        {
            muteDelay += 50;
        }
        bUsed = true;
    }
    else if( bCCW )
    {
        if( muteDelay > 50 )
        {
            muteDelay -= 50;
        }
        else if( muteDelay > 0 )
        {
            muteDelay--;
        }
        bUsed = true;
    }
    else if( bShortPress )
    {
        muteDelay = 5;
        bUsed = true;
    }
    char buf[TEXT_BUF_LEN];
    sprintf( buf, "Mute dly: %d", muteDelay);
    displayText( MENU_LINE, buf, true );
    
    return bUsed;
}

// Menu for changing the crystal frequency
// Each digit can be changed individually
static bool menuXtalFreq( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // When the menu is entered we are changing the first changeable digit of the
    // crystal frequency i.e. MHz. This is digit 7.
    #define INITIAL_FREQ_CHANGE 1000000
    #define INITIAL_FREQ_POS    7

    // The final changeable digit position
    #define FINAL_FREQ_POS      13

    // Set to true as expecting to use the presses etc
    bool bUsed = true;
    
    // The current change in frequency
    static uint32_t freqChange;
    
    // The new frequency starts with the current value
    // Will only want to update it if they have changed
    static uint32_t oldFreq, newFreq;

    // If just entered the menu...
    if( !bCW && !bCCW && !bShortPress &&!bLongPress && !bShortPressLeft &&!bShortPressRight )
    {
        // Start with the current xtal frequency from NVRAM
        newFreq = oldFreq = nvramReadXtalFreq();
        
        // Start with changing the first digit
        // i.e. the tens of MHz
        freqChange = INITIAL_FREQ_CHANGE;
        xtalFreqPos = INITIAL_FREQ_POS;
        
        // Set the cursor on the digit to be changed
        displayCursor( xtalFreqPos, MENU_LINE, cursorUnderline );

        // We aren't asking to save the new frequency to NVRAM just yet
        bAskToSaveXtalFreq = false;
    }

    // We are asking whether or not to save the new value to NVRAM
    if( bAskToSaveXtalFreq )
    {
        // A long press means we are quitting without saving
        if( bLongPress )
        {
            // Set back the original frequency
            oscSetXtalFrequency( nvramReadXtalFreq() );

            // Retune to use the xtal frequency
            setFrequencies();

            bUsed = false;
        }
        // A short press means we are writing the new frequency
        // and quitting
        else if( bShortPress )
        {
            // Write the new crystal frequency to NVRAM
            nvramWriteXtalFreq( newFreq );
            
            // Take us out of the menu item
            enterVFOMode();

            bUsed = false;
        }
    }
    else
    {
        // If a long press then we are ending without
        // writing new value to NVRAM
        if( bLongPress )
        {
            // Don't want the cursor any more
            displayCursor( 0, 0, cursorOff );

            // Set back the original frequency
            oscSetXtalFrequency( nvramReadXtalFreq() );

            // Retune to use the xtal frequency
            setFrequencies();

            // This ensures the long press is processed to quit
            bUsed = false;
        }
        // If a short press then we are ending - need to see if we should
        // write new value to NVRAM
        else if( bShortPress )
        {
            // Has the frequency changed?
            if( newFreq != oldFreq )
            {
                // Yes, so see if user wants to save
                bAskToSaveXtalFreq = true;
                
                // Don't want the cursor any more
                displayCursor( 0, 0, cursorOff );

                displayText( MENU_LINE, "Short press to save", true );
            }
        }
        else
        {
            // Rotation is a change up or down in frequency
            if( bCW )
            {
                // Clock wise so increasing in frequency provided we aren't
                // going to go over the maximum
                if( newFreq <= (MAX_XTAL_FREQUENCY - freqChange) )
                {
                    newFreq += freqChange;
                }
            }
            else if( bCCW )
            {
                // Counter clockwise so decreasing in frequency provided we
                // aren't going to go under the minimum
                if( newFreq >= (MIN_XTAL_FREQUENCY + freqChange) )
                {
                    newFreq -= freqChange;
                }
            }

            // A short left or right press changes the rate we tune at
            if( bShortPressRight )
            {
                if( freqChange == 1 )
                {
                    // Currently at the lowest position so start at the top
                    // again
                    freqChange = INITIAL_FREQ_CHANGE;
                    xtalFreqPos = INITIAL_FREQ_POS;
                }
                else
                {
                    // Now changing the next digit
                    freqChange /= 10;
                    xtalFreqPos++;
                }

                // Set the cursor to the correct position for the current amount of change
                displayCursor( xtalFreqPos, MENU_LINE, cursorUnderline );
            }
            else if( bShortPressLeft )
            {
                if( freqChange == INITIAL_FREQ_CHANGE )
                {
                    // Currently at the highest position so start at the bottom
                    // again
                    freqChange = 1;
                    xtalFreqPos = FINAL_FREQ_POS;
                }
                else
                {
                    // Now changing the next digit
                    freqChange *= 10;
                    xtalFreqPos--;
                }

                // Set the cursor to the correct position for the current amount of change
                displayCursor( xtalFreqPos, MENU_LINE, cursorUnderline );
            }

            // Display the current frequency
            char buf[TEXT_BUF_LEN];
            sprintf( buf, "Xtal: %0lu", newFreq);
            displayText( MENU_LINE, buf, true );

            // If the frequency is to change...
            if( newFreq != oldFreq )
            {
                // Set it in the oscillator driver
                oscSetXtalFrequency( newFreq );
                
                // Set the RX and TX frequencies again - this will pick up the
                // new crystal frequency
                setFrequencies();
            }
        }
    }
    return bUsed;
}

static bool menuKeyerMode( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // Set to true if we have used the presses etc
    bool bUsed = false;
    
    // Current keyer mode
    static enum eMorseKeyerMode keyerMode;
    
    // If just entered the menu get the current mode
    if( !bCW && !bCCW && !bShortPress &&!bLongPress )
    {
        keyerMode = morseGetKeyerMode();
    }

    // Left or right button presses
    if( bShortPressRight )
    {
        keyerMode++;
        if( keyerMode == MORSE_NUM_KEYER_MODES)
        {
            keyerMode = 0;
        }
    }
    else if( bShortPressLeft )
    {
        if( keyerMode == 0 )
        {
            keyerMode = MORSE_NUM_KEYER_MODES - 1;
        }
        else
        {
            keyerMode--;
        }
    }

    switch( keyerMode )
    {
        case morseKeyerIambicA:
            displayText( MENU_LINE, "Keyer: Iambic A", true );
            break;

        case morseKeyerIambicB:
            displayText( MENU_LINE, "Keyer: Iambic B", true );
            break;

        case morseKeyerUltimatic:
        default:
            displayText( MENU_LINE, "Keyer: Ultimatic", true );
            break;
    }
    
    if( bShortPress )
    {
        // Short press writes the new mode
        morseSetKeyerMode( keyerMode );

        // Also write to NVRAM
        nvramWriteMorseKeyerMode( keyerMode );
        
        // Leave the menu and go back to VFO mode
        enterVFOMode();
    }

    return bUsed;
}

// Gets a VFO frequency - usually called from CAT control
uint32_t getVFOFreq( uint8_t vfo )
{
    uint32_t freq = 0;
    if( vfo < NUM_VFOS )
    {
        freq = vfoState[vfo].freq;
    }
    return freq;
}

// Get the current VFO frequency
uint32_t getCurrentVFOFreq()
{
    return getVFOFreq( currentVFO );
}

// Get the other VFO frequency
uint32_t getOtherVFOFreq()
{
    return getVFOFreq( OTHER_VFO );
}

// Get current VFO RIT/XIT offset
int16_t getCurrentVFOOffset()
{
    return vfoState[currentVFO].offset;
}

// Returns true if the current VFO is in RIT mode
bool getCurrentVFORIT()
{
    return (vfoState[currentVFO].mode == vfoRIT);
}

// Returns true if the current VFO is in XIT mode
bool getCurrentVFOXIT()
{
    return (vfoState[currentVFO].mode == vfoXIT);
}

// Get other VFO RIT/XIT offset
int16_t getOtherVFOOffset()
{
    return vfoState[currentVFO].offset;
}

// Returns true if the other VFO is in RIT mode
bool getOtherVFORIT()
{
    return (vfoState[currentVFO].mode == vfoRIT);
}

// Returns true if the other VFO is in XIT mode
bool getOtherVFOXIT()
{
    return (vfoState[currentVFO].mode == vfoXIT);
}

// Sets a VFO to a frequency - usually called from CAT control
void setVFOFrequency( uint8_t vfo, uint32_t freq )
{
    if( vfo < NUM_VFOS )
    {
        vfoState[vfo].freq = freq;
    }
    setFrequencies();
}

// Return the current VFO - for CAT control
uint8_t getCurrentVFO()
{
    return currentVFO;
}

// Set the current VFO - from CAT control
void setCurrentVFO( uint8_t vfo )
{
    if( vfo < NUM_VFOS )
    {
        currentVFO = vfo;

        // Update the frequencies and display
        setFrequencies();
        update_display();
    }
}

// Get the split state - for CAT control
bool getVFOSplit()
{
    return bVFOSplit;
}

// Get the transmitting state - for CAT control
bool getTransmitting()
{
    return bTransmitting;
}
#endif

// Adjust a VFO. Changes the frequency or the offset by the supplied change.
// Could change both but not a normal usage (simplex changes the frequency, 
// RIT and XIT change the offset 
static void adjustVFO( uint8_t vfo, int16_t freqChange, int16_t offsetChange )
{
    // Record the new frequency and offset
    vfoState[vfo].freq = vfoState[vfo].freq + freqChange;
    vfoState[vfo].offset = vfoState[vfo].offset + offsetChange;

    // Set the new TX and RX frequencies
    setFrequencies();
}

// Handle the rotary control while in the VFO mode
#ifdef SOTA2
static void rotaryVFO( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // How much to change frequency by
    int16_t change = SOTA2_FREQ_CHANGE;
    
    // Rotation is a change up or down in frequency
    if( bCW )
    {
        // Leave change as is (+ve)
    }
    else if( bCCW )
    {
        // Counter clockwise means change is -ve
        change = -change;
    }
    else
    {
        // Control not moved so no change
        change = 0;
    }

    if( bShortPress )
    {
        // A short press takes us back to the home frequency for the current band
        setBand( currentBand );
    }
    else if( bLongPress )
    {
        // A long press changes band
        setBand( (currentBand+1)%NUM_BANDS );
    }
    else
    {
        adjustVFO( currentVFO, change, 0);
    }
}

#else
static void rotaryVFO( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // How much to change frequency by
    int16_t change;
    
    // Set the amount each click changes VFO by
    change = vfoCursorTransition[cursorIndex].freqChange;

    // Rotation is a change up or down in frequency
    if( bCW )
    {
        // Leave change as is (+ve)
    }
    else if( bCCW )
    {
        // Counter clockwise means change is -ve
        change = -change;
    }
    else
    {
        // Control not moved so no change
        change = 0;
    }

    if( bShortPress )
    {
        // In split mode change between lines
        if( bVFOSplit )
        {
            // Change to the other line and display the cursor on this line
            if( bVFOFirstLine )
            {
                bVFOFirstLine = false;
            }
            else
            {
                bVFOFirstLine = true;
            }
        }
        else if( vfoState[currentVFO].mode == vfoSimplex )
        {
            // In simplex mode a short press enters the band setting menu
            enterVFOBandMenu();
        }
        else
        {
            // In RIT and XIT modes a short press zeroes the offset
            setCurrentVFOOffset( 0 );
        }

        // Display the cursor in the correct place
        update_cursor();
    }
    else if( bLongPress )
    {
        // A long press takes us to the menu
        enterMenu();
    }
    else if( bShortPressLeft )
    {
        cursorIndex++;
        if( vfoCursorTransition[cursorIndex].x == CURSOR_TRANSITION_END )
        {
            cursorIndex = 0;
        }
        update_cursor();
    }
    else if( bLongPressLeft )
    {
        if( vfoState[currentVFO].mode == vfoSimplex )
        {
            // A long left press takes us to the quick menu in simplex
            enterQuickMenu();
        }
        else
        {
            // Otherwise it takes us back to simplex
            enterSimplex();
        }
    }
    else if( bShortPressRight )
    {
        if( cursorIndex == 0 )
        {
            cursorIndex = NUM_CURSOR_TRANSITIONS - 2;
        }
        else
        {
            cursorIndex--;
        }
        update_cursor();
    }
    else if( bLongPressRight )
    {
        // A long right press takes us to WPM
        enterWpm();
    }
    else
    {
        // Adjust the VFO
        if( bVFOSplit )
        {
            // In split mode so adjust the VFO according to which line
            // we are on
            if( bVFOFirstLine )
            {
                adjustVFO( currentVFO, change, 0);
            }
            else
            {
                adjustVFO( OTHER_VFO, change, 0);
            }
        }
        // Not in split mode
        else if( vfoState[currentVFO].mode == vfoSimplex )
        {
            // Simplex so change the frequency
            adjustVFO( currentVFO, change, 0);
        }
        else
        {
            // RIT or XIT so adjust the offset
            adjustVFO( currentVFO, 0, change );
        }
    }
}

// Set the RIT - called from CAT control
void setCurrentVFOOffset( int16_t rit )
{
    // Set the RIT by changing the offset.
    adjustVFO( currentVFO, 0, rit-vfoState[currentVFO].offset);
}

// Handle the rotary control while in the wpm setting mode
static void rotaryWpm( bool bCW, bool bCCW, bool bShortPress, bool bLongPress, bool bShortPressLeft, bool bLongPressLeft, bool bShortPressRight, bool bLongPressRight )
{
    // When switching into straight key mode want to remember current wpm
    // so that we switch back to that instead of the default
    static uint8_t previousWpm = DEFAULT_MORSE_WPM;

    // The new wpm starts with the current value
    // Will only want to update them if they have changed
    uint8_t newWpm;
    uint8_t morseWpm;
    newWpm = morseWpm = morseGetWpm();

    // Only change it if not in straight key mode
    if( newWpm )
    {
        if( bCW )
        {
            newWpm++;
        }
        else if( bCCW )
        {
            newWpm--;
        }
    }

    // A short press takes us to straight key mode
    if( bShortPress )
    {
        if( newWpm )
        {
            // Remember the current wpm setting
            previousWpm = morseWpm;

            // Wpm of 0 means straight key mode
            newWpm = 0;
        }
        else
        {
            // Already in straight key mode
            // If in tune mode, stop transmitting
            if( morseInTuneMode() )
            {
                morseSetTuneMode( false );
            }
            else
            {
                // Otherwise out of straight key mode
                newWpm = previousWpm;
            }
        }
    }
    // A long press enters the menu
    else if( bLongPress )
    {
        enterMenu();
    }
    // A right press puts us back to VFO mode
    else if( bShortPressRight )
    {
        enterVFOMode();
    }
    // A left press puts us in the quick menu
    else if( bShortPressLeft )
    {
        enterQuickMenu();
    }

    // If the wpm is to change then check it is in range before
    // setting it
    if( newWpm != morseWpm )
    {
        // wpm of 0 is straight key mode so no need to check
        if( newWpm != 0 )
        {
            if( newWpm > MAX_MORSE_WPM )
            {
                newWpm = MAX_MORSE_WPM;
            }
            else if( newWpm < MIN_MORSE_WPM )
            {
                newWpm = MIN_MORSE_WPM;
            }
        }
        morseWpm = newWpm;
        morseSetWpm( newWpm );
        
        // Write the new morse speed to NVRAM
        nvramWriteWpm( newWpm );

        // Display the new speed
        update_display();
    }
}
#endif

// See if the rotary control has been touched and handle its movement
// This will update either the VFO or the wpm or the menu
// Also handles the left and right buttons
static void handleRotary()
{
    bool bShortPress;
    bool bLongPress;
    bool bCW;
    bool bCCW;
    bool bShortPressLeft;
    bool bLongPressLeft;
    bool bShortPressRight;
    bool bLongPressRight;

    // Button debounce states
    static struct sDebounceState debounceStateLeft, debounceStateRight;

    // Debounce the pushbuttons
    debouncePushbutton( ioReadLeftButton(),  &bShortPressLeft,  &bLongPressLeft,  DEBOUNCE_TIME, LONG_PRESS_TIME, &debounceStateLeft);
    debouncePushbutton( ioReadRightButton(), &bShortPressRight, &bLongPressRight, DEBOUNCE_TIME, LONG_PRESS_TIME, &debounceStateRight);

    // Read the rotary state
    readRotary(&bCW, &bCCW, &bShortPress, &bLongPress);

    // Call the handler if anything has happened
    if( bCW || bCCW || bShortPress || bLongPress || bShortPressLeft || bLongPressLeft || bShortPressRight || bLongPressRight )
    {
#ifdef SOTA2
        rotaryVFO(bCW, bCCW, bShortPress, bLongPress, bShortPressLeft, bLongPressLeft, bShortPressRight, bLongPressRight);
#else
        switch( currentMode )
        {
            case modeMenu:
                rotaryMenu(bCW, bCCW, bShortPress, bLongPress, bShortPressLeft, bLongPressLeft, bShortPressRight, bLongPressRight);
                break;

            case modeQuickMenu:
                rotaryQuickMenu(bCW, bCCW, bShortPress, bLongPress, bShortPressLeft, bLongPressLeft, bShortPressRight, bLongPressRight);
                break;

            case modeWpm:
                rotaryWpm(bCW, bCCW, bShortPress, bLongPress, bShortPressLeft, bLongPressLeft, bShortPressRight, bLongPressRight);
                break;

            default:
            case modeVFO:
            case modeFastVFO:
                rotaryVFO(bCW, bCCW, bShortPress, bLongPress, bShortPressLeft, bLongPressLeft, bShortPressRight, bLongPressRight);
                break;
        }
#endif
    }
}


// Main loop is called repeatedly
static void loop()
{
    // See if the morse paddles or straight key have been pressed
    // If not active then deal with other things too
	if( !morseScanPaddles() )
    {
        // Deal with the rotary control/pushbutton
        handleRotary();

#ifndef SOTA2
        // Do CAT control
        catControl();
#endif
    }
}

int main(void)
{
    // Disable the clock prescaler so that it runs at 20MHz
    // Have to enable access first
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = 0;

    // Start the millisecond timer - it enables timer interrupts
    millisInit();

    // Initialise the input/output module
    ioInit();

    // Initialise the NVRAM/EEPROM before other modules as it contains values needed for other setup
    nvramInit();

#ifndef SOTA2
    // Initialise CAT control
    catInit();
#endif

    // Set up morse and set the speed and keyer mode as read from NVRAM
    morseInit();
    morseSetWpm( nvramReadWpm() );
    morseSetKeyerMode( nvramReadMorseKeyerMode() );

#ifndef SOTA2
    displayInit();
#endif
   
    // Initialise the oscillator chip
	bOscInit = oscInit();

    // Load the crystal frequency from NVRAM
    oscSetXtalFrequency( nvramReadXtalFreq() );

    // Set the band from the NVRAM
    // This also updates the display with frequency and wpm.
#ifdef SOTA2
    setBand( DEFAULT_BAND );
#else
    setBand( nvramReadBand() );
#endif

    // Enable the RX clock outputs
    // We enable the TX output only when transmitting
    enableRXClock( true );

    while (1) 
    {
        loop();
    }
}
