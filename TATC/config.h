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

// Each click changes the frequency by this amount
#define SOTA2_FREQ_CHANGE 100

#ifdef VPORTC

// ATtiny 1-series

// Processor definitions
// CPU clock speed
#define F_CPU 20000000UL

// Clock divider
// This is used by the millisecond clock on TCA0
// This also feeds clock TCB1 which is used to generate sidetone
// and so needs to be divided down enough to get an audible frequency
#define CLOCK_DIV 256

// I/O definitions

// Pushbuttons to use the ADC to save pins
#define ANALOGUE_BUTTONS

// Pushbutton inputs
// Either connected to an ADC input
// or to GPIO pins.
#ifdef ANALOGUE_BUTTONS

// ADC used for the buttons with input channel,
// pin control register and interrupt vector
#define BUTTON_ADC              ADC1
#define BUTTON_ADC_CHAN         ADC_MUXPOS_AIN6_gc
#define BUTTON_ADC_PINCTRL      PORTC.PIN0CTRL
#define BUTTON_ADC_RESRDY_vect  ADC1_RESRDY_vect

// ADC values for the left, right and rotary buttons.
#define ROTARY_SW_MIN 0
#define ROTARY_SW_MAX 49
#define LEFT_BUTTON_MIN 50
#define LEFT_BUTTON_MAX 110
#define RIGHT_BUTTON_MIN 110
#define RIGHT_BUTTON_MAX 220

#else

#define RIGHT_DIR_REG      VPORTC.DIR
#define RIGHT_IN_REG       VPORTC.IN
#define RIGHT_PIN          0
#define RIGHT_PIN_CTRL     PORTC.PIN0CTRL

#define LEFT_DIR_REG      VPORTB.DIR
#define LEFT_IN_REG       VPORTB.IN
#define LEFT_PIN          2
#define LEFT_PIN_CTRL     PORTB.PIN2CTRL

#define ROTARY_ENCODER_SW_DIR_REG   VPORTC.DIR
#define ROTARY_ENCODER_SW_IN_REG    VPORTC.IN
#define ROTARY_ENCODER_SW_PIN       1
#define ROTARY_ENCODER_SW_PIN_CTRL  PORTC.PIN1CTRL

#endif

#define ROTARY_ENCODER_A_DIR_REG    VPORTC.DIR
#define ROTARY_ENCODER_A_IN_REG     VPORTC.IN
#define ROTARY_ENCODER_A_PIN        2
#define ROTARY_ENCODER_A_PIN_CTRL   PORTC.PIN2CTRL

#define ROTARY_ENCODER_B_DIR_REG    VPORTC.DIR
#define ROTARY_ENCODER_B_IN_REG     VPORTC.IN
#define ROTARY_ENCODER_B_PIN        3
#define ROTARY_ENCODER_B_PIN_CTRL   PORTC.PIN3CTRL

#define MORSE_PADDLE_DASH_DIR_REG    VPORTA.DIR
#define MORSE_PADDLE_DASH_IN_REG     VPORTA.IN
#define MORSE_PADDLE_DASH_PIN        4
#define MORSE_PADDLE_DASH_PIN_CTRL   PORTA.PIN4CTRL

#define MORSE_PADDLE_DOT_DIR_REG    VPORTA.DIR
#define MORSE_PADDLE_DOT_IN_REG     VPORTA.IN
#define MORSE_PADDLE_DOT_PIN        5
#define MORSE_PADDLE_DOT_PIN_CTRL   PORTA.PIN5CTRL

#define MORSE_OUTPUT_DIR_REG     VPORTA.DIR
#define MORSE_OUTPUT_OUT_REG     VPORTA.OUT
#define MORSE_OUTPUT_PIN         6

#define RX_ENABLE_DIR_REG     VPORTA.DIR
#define RX_ENABLE_OUT_REG     VPORTA.OUT
#define RX_ENABLE_PIN         7

#ifdef SOTA2

// The number of band relays
#define NUM_RELAYS 1

#define RELAY_0_OUTPUT_DIR_REG     VPORTC.DIR
#define RELAY_0_OUTPUT_OUT_REG     VPORTC.OUT
#define RELAY_0_OUTPUT_PIN         1

// Front panel LEDs
#define RIGHT_LED_OUTPUT_DIR_REG   VPORTB.DIR
#define RIGHT_LED_OUTPUT_OUT_REG   VPORTB.OUT
#define RIGHT_LED_OUTPUT_PIN       2

#define CENTRE_LED_OUTPUT_DIR_REG  VPORTB.DIR
#define CENTRE_LED_OUTPUT_OUT_REG  VPORTB.OUT
#define CENTRE_LED_OUTPUT_PIN      3

#define LEFT_LED_OUTPUT_DIR_REG    VPORTB.DIR
#define LEFT_LED_OUTPUT_OUT_REG    VPORTB.OUT
#define LEFT_LED_OUTPUT_PIN        4

#else

// The number of band relays
#define NUM_RELAYS 5

#define RELAY_0_OUTPUT_DIR_REG     VPORTB.DIR
#define RELAY_0_OUTPUT_OUT_REG     VPORTB.OUT
#define RELAY_0_OUTPUT_PIN         3

#define RELAY_1_OUTPUT_DIR_REG     VPORTB.DIR
#define RELAY_1_OUTPUT_OUT_REG     VPORTB.OUT
#define RELAY_1_OUTPUT_PIN         4

#define RELAY_2_OUTPUT_DIR_REG     VPORTB.DIR
#define RELAY_2_OUTPUT_OUT_REG     VPORTB.OUT
#define RELAY_2_OUTPUT_PIN         5

#define RELAY_3_OUTPUT_DIR_REG     VPORTC.DIR
#define RELAY_3_OUTPUT_OUT_REG     VPORTC.OUT
#define RELAY_3_OUTPUT_PIN         1

#define RELAY_4_OUTPUT_DIR_REG     VPORTB.DIR
#define RELAY_4_OUTPUT_OUT_REG     VPORTB.OUT
#define RELAY_4_OUTPUT_PIN         2

#endif

// Oscillator chip definitions
// I2C address
#define SI5351A_I2C_ADDRESS 0x60

// The si5351a default crystal frequency and load capacitance
#define DEFAULT_XTAL_FREQ	25000000UL
#define SI_XTAL_LOAD_CAP SI_XTAL_LOAD_8PF

#define I2C_CLOCK_RATE 100000

// Define to use the alternative pins (PA1, PA2) instead
// of the standard pins (PB2,PB3)
#define SERIAL_ALTERNATIVE_PINS
#ifdef SERIAL_ALTERNATIVE_PINS
#define SERIAL_DIR_REG  VPORTA.DIR
#define SERIAL_TXD_PIN  1
#define SERIAL_RXD_PIN  2
#else
#define SERIAL_DIR_REG  VPORTB.DIR
#define SERIAL_TXD_PIN  2
#define SERIAL_RXD_PIN  3
#endif

// Using I2C LCD
#define LCD_I2C

// Address of the LCD display
#define LCD_I2C_ADDRESS 0x27

#else

// Processor definitions
// CPU clock speed
#define F_CPU 16000000UL

// The si5351a default crystal frequency and load capacitance
#define DEFAULT_XTAL_FREQ	27000000
#define SI_XTAL_LOAD_CAP SI_XTAL_LOAD_10PF

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
#define RX_ENABLE_PORT_REG      PORTC
#define RX_ENABLE_PIN_REG       PINC
#define RX_ENABLE_DDR_REG       DDRC
#define RX_ENABLE_PIN           PC3

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
#define LCD_PORT
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

#endif

// Position of default band in the frequency table defined in main.c
// The band is stored in NVRAM so this is only used on first power up
#ifdef SOTA2
#define DEFAULT_BAND 0
#else
#define DEFAULT_BAND 3
#endif

// By default we are not using CW-Reverse mode
#define DEFAULT_CWREVERSE false

// Serial port definitions
#define SERIAL_BAUD 57600

// Time between scans of the CAT interface
#define CAT_CHARACTER_DELAY 10

// Oscillator chip definitions
// I2C address
#define SI5351A_I2C_ADDRESS 0x60

// Transmit and receive clocks. Receive uses 2 clocks for quadrature.
#define NUM_CLOCKS 3
#define RX_CLOCK_A 0
#define RX_CLOCK_B 1
#define TX_CLOCK   2

// The minimum and maximum crystal frequencies in the setting menu
// Have to allow for adjusting above or below actual valid crystal range
#define MIN_XTAL_FREQUENCY 24000000
#define MAX_XTAL_FREQUENCY 28000000

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

#ifdef SOTA2

#define RELAY_STATE_40M    0        // This means relay off
#define TX_ENABLED_40M     true

#define RELAY_STATE_20M    1
#define TX_ENABLED_20M     true

#else

#define RELAY_STATE_160M   0
#define TX_ENABLED_160M    false
#define QUICK_VFO_160M     false

#define RELAY_STATE_80M    0
#define TX_ENABLED_80M     true
#define QUICK_VFO_80M      true

#define RELAY_STATE_60M    3
#define TX_ENABLED_60M     true
#define QUICK_VFO_60M      true

#define RELAY_STATE_40M    1
#define TX_ENABLED_40M     true
#define QUICK_VFO_40M      true

#define RELAY_STATE_30M    2
#define TX_ENABLED_30M     true
#define QUICK_VFO_30M      true

#define RELAY_STATE_20M    4
#define TX_ENABLED_20M     true
#define QUICK_VFO_20M      true

#define RELAY_STATE_17M    4
#define TX_ENABLED_17M     false
#define QUICK_VFO_17M      false

#define RELAY_STATE_15M    4
#define TX_ENABLED_15M     false
#define QUICK_VFO_15M      false

#define RELAY_STATE_12M    4
#define TX_ENABLED_12M     false
#define QUICK_VFO_12M      false

#define RELAY_STATE_10M    4
#define TX_ENABLED_10M     false
#define QUICK_VFO_10M      false

#endif

// Time for debouncing a button (ms)
#define DEBOUNCE_TIME   100

// Time for a button press to be a long press (ms)
#define LONG_PRESS_TIME 250

// Buffer lengths for the serial port
// Length should be a power of 2 for efficiency
#define SERIAL_RX_BUF_LEN 32
#define SERIAL_TX_BUF_LEN 64

// Time for debouncing the rotary pushbutton (ms)
#define ROTARY_BUTTON_DEBOUNCE_TIME   100

// Time for the rotary pushbutton to be a long press (ms)
#define ROTARY_LONG_PRESS_TIME 250

#endif /* CONFIG_H_ */
