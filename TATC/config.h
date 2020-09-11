/*
 * config.h
 * 
 * Configuration settings for the whole project
 * e.g. hardware settings, screen size, morse preferences
 *
 * Created: 04/12/2018 20:05:26
 *  Author: Richard
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#include <avr/io.h>

// General definitions
typedef uint8_t bool;
#define true 1
#define false 0

#define ULONG_MAX 0xFFFFFFFF

// Position of default band in the frequency table defined in main.c
// The band is stored in NVRAM so this is only used on first power up
#define DEFAULT_BAND 3

// By default we are not using CW-Reverse mode
#define DEFAULT_CWREVERSE false

// Processor definitions
// CPU clock speed
#define F_CPU 16000000UL

// Serial port definitions
#define SERIAL_BAUD 57600

// Time between scans of the CAT interface
#define CAT_CHARACTER_DELAY 10

// Oscillator chip definitions
// I2C address
#define SI5351A_I2C_ADDRESS 0x62

// Transmit and receive clocks. Receive uses 2 clocks for quadrature.
#define NUM_CLOCKS 3
#define RX_CLOCK_A 0
#define RX_CLOCK_B 1
#define TX_CLOCK   2

// The minimum and maximum crystal frequencies in the setting menu
// Have to allow for adjusting above or below actual valid crystal range
#define MIN_XTAL_FREQUENCY 24000000
#define MAX_XTAL_FREQUENCY 28000000

// The si5351a default crystal frequency and load capacitance
#define DEFAULT_XTAL_FREQ	27000000
#define SI_XTAL_LOAD_CAP SI_XTAL_LOAD_10PF

// Morse definitions
// Frequency of CW tone
#define CW_FREQUENCY 700

// The receive frequency needs to be offset from the dial frequency
// to receive with the correct tone.
#define RX_OFFSET CW_FREQUENCY

// Default, minimum and maximum morse speed in wpm
#define DEFAULT_MORSE_WPM 18
#define MIN_MORSE_WPM 5
#define MAX_MORSE_WPM 40

// Default morse keyer mode
#define DEFAULT_KEYER_MODE 0

// Morse paddle inputs
#define MORSE_PADDLE_DOT_PORT_REG   PORTB
#define MORSE_PADDLE_DOT_PIN_REG    PINB
#define MORSE_PADDLE_DOT_PIN        PB1
#define MORSE_PADDLE_DASH_PORT_REG  PORTB
#define MORSE_PADDLE_DASH_PIN_REG   PINB
#define MORSE_PADDLE_DASH_PIN       PB0

// I/O definitions
// Output pins
#define RELAY_OUTPUT_PORT_REG     PORTD
#define RELAY_OUTPUT_PIN_REG      PIND
#define RELAY_OUTPUT_DDR_REG      DDRD
#define RELAY_OUTPUT_PIN          PD5
#define MORSE_OUTPUT_PORT_REG   PORTB
#define MORSE_OUTPUT_PIN_REG    PINB
#define MORSE_OUTPUT_DDR_REG    DDRB
#define MORSE_OUTPUT_PIN        PB2
#define TX_OUTPUT_PORT_REG      PORTC
#define TX_OUTPUT_PIN_REG       PINC
#define TX_OUTPUT_DDR_REG       DDRC
#define TX_OUTPUT_PIN           PC3

// Rotary encoder to use the ADC to save pins
#define ROTARY_ANALOGUE

// Rotary encoder inputs
// Either connected to an ADC input
// or to GPIO pins.
#ifdef ROTARY_ANALOGUE

// ADC values for the rotary A, B and switch outputs.
#define ROTARY_A_MIN 600
#define ROTARY_A_MAX 800
#define ROTARY_B_MIN 0
#define ROTARY_B_MAX 500
#define ROTARY_SW_MIN 150
#define ROTARY_SW_MAX 250

#else

// Define each GPIO pin being used for the rotary control
#define ROTARY_ENCODER_A_PORT_REG   PORTC
#define ROTARY_ENCODER_A_PIN_REG    PINC
#define ROTARY_ENCODER_A_DDR_REG    DDRC
#define ROTARY_ENCODER_A_PIN        PC1
#define ROTARY_ENCODER_B_PORT_REG   PORTC
#define ROTARY_ENCODER_B_PIN_REG    PINC
#define ROTARY_ENCODER_B_DDR_REG    DDRC
#define ROTARY_ENCODER_B_PIN        PC2
#define ROTARY_ENCODER_SW_PORT_REG  PORTC
#define ROTARY_ENCODER_SW_PIN_REG   PINC
#define ROTARY_ENCODER_SW_DDR_REG   DDRC
#define ROTARY_ENCODER_SW_PIN       PC0

#endif

// Display definitions
// LCD Port definitions
#define LCD_ENABLE_PORT PORTB
#define LCD_ENABLE_DDR  DDRB
#define LCD_ENABLE_PIN  PB3
#define LCD_RS_PORT     PORTB
#define LCD_RS_DDR      DDRB
#define LCD_RS_PIN      PB4
#define LCD_DATA_PORT_0 PORTD
#define LCD_DATA_DDR_0  DDRD
#define LCD_DATA_PIN_0  PD7
#define LCD_DATA_PORT_1  PORTD
#define LCD_DATA_DDR_1  DDRD
#define LCD_DATA_PIN_1  PD4
#define LCD_DATA_PORT_2  PORTD
#define LCD_DATA_DDR_2  DDRD
#define LCD_DATA_PIN_2  PD3
#define LCD_DATA_PORT_3  PORTD
#define LCD_DATA_DDR_3  DDRD
#define LCD_DATA_PIN_3  PD2

// How often to update the display
#define DISPLAY_INTERVAL 50

// Dimensions of the LCD screen
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

// Length of text buffers
// Set to the be the LCD line length with a safety margin
#define TEXT_BUF_LEN (LCD_WIDTH*2)

// Lines on the display
#define FREQ_LINE  0
#define MENU_LINE  1
#define MORSE_LINE 1
#define WPM_LINE   0

// The position on the screen of the morse WPM setting cursor
#define WPM_COL 12

#define ENABLE_DISPLAY_SPLIT_LINE

// For each band, what state should the LPF/PA relay be in
// and is TX allowed on the band

// Currently, the relay state is not used
// See RELAY_ON_FREQ below.

#define RELAY_STATE_160M   1
#define TX_ENABLED_160M    false

#define RELAY_STATE_80M    1
#define TX_ENABLED_80M     true

#define RELAY_STATE_60M    0
#define TX_ENABLED_60M     false

#define RELAY_STATE_40M    0
#define TX_ENABLED_40M     false

#define RELAY_STATE_30M    0
#define TX_ENABLED_30M     false

#define RELAY_STATE_20M    0
#define TX_ENABLED_20M     true

#define RELAY_STATE_17M    0
#define TX_ENABLED_17M     false

#define RELAY_STATE_15M    0
#define TX_ENABLED_15M     false

#define RELAY_STATE_12M    0
#define TX_ENABLED_12M     false

#define RELAY_STATE_10M    0
#define TX_ENABLED_10M     false

// The frequency below which the relay is switched on
#define RELAY_ON_FREQ      4000000

// Time for debouncing a switch (ms)
#define DEBOUNCE_TIME   100

// Time for a key press to be a long press (ms)
#define LONG_PRESS_TIME 250

// Buffer lengths for the serial port
// Length should be a power of 2 for efficiency
#define SERIAL_RX_BUF_LEN 32
#define SERIAL_TX_BUF_LEN 64

// Time for debouncing a switch (ms)
#define ROTARY_BUTTON_DEBOUNCE_TIME   100

// Time for a key press to be a long press (ms)
#define ROTARY_LONG_PRESS_TIME 250

#endif /* CONFIG_H_ */
