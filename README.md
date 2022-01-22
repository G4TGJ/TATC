# TATC
Software for the [G4TGJ 5 Band CW QRP Transceiver](https://g4tgj.github.io/5-Band-CW-QRP-Transceiver/) and the [Two Band CW QRP SOTA Transceiver](https://g4tgj.github.io/Two-Band-CW-QRP-SOTA-Transceiver/).

## Building the sofware

To compile from source you will need this repo and [TARL](https://github.com/G4TGJ/TARL).

### Windows Build

You can download the source as zip files or clone the repo using git. To do this install [Git for Windows](https://git-scm.com/download/win) or 
[GitHub Desktop](https://desktop.github.com/).

To build with [Atmel Studio 7](https://www.microchip.com/mplab/avr-support/atmel-studio-7) open TATC.atsln.

From Build/Configuration Manager you can select 5Band or Sota2 to select which version you wish to build.

### Linux Build

To build with Linux you will need to install git, the compiler and library. For Ubuntu:

    sudo apt install gcc-avr avr-libc git
    
    
Clone this repo plus TARL:

    git clone https://github.com/G4TGJ/TATC.git
    git clone https://github.com/G4TGJ/TARL.git
    
Build for the 5 Band transceiver:

    cd TATC/TATC
    ./build.sh

This creates 5Band/TATC.hex.

Build for the Two Band SOTA transceiver:

    cd TATC/TATC
    ./buildsota2.sh

This creates Sota2/TATC.hex.

TATC stands for 'TGJ AVR Transceiver Controller.
