# TATC
Software for the G4TGJ 5 Band CW QRP Transceiver.

## Building the sofware

To compile from source you will need this repo and [TARL](https://github.com/G4TGJ/TARL).

### Windows Build

You can download the source as zip files or clone the repo using git. To do this install [Git for Windows](https://git-scm.com/download/win) or 
[GitHub Desktop](https://desktop.github.com/).

To build with [Atmel Studio 7](https://www.microchip.com/mplab/avr-support/atmel-studio-7) open TATC.atsln.

### Linux Build

To build with Linux you will need to install git, the compiler and library. For Ubuntu:

    sudo apt install gcc-avr avr-libc git
    
    
Clone this repo plus TARL:

    git clone https://github.com/G4TGJ/TATC.git
    git clone https://github.com/G4TGJ/TARL.git
    
Build:

    cd TATC/TATC
    ./build.sh

This creates Release/TATC.hex.

TATC stands for 'TGJ AVR Transceiver Controller.
