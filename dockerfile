FROM opthomasprime/avr-gcc

RUN git clone https://github.com/G4TGJ/TARL.git

COPY TATC TATC

WORKDIR TATC

