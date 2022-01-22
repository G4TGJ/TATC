#!/bin/sh
./convert.sh 5Band
cd 5Band
sed -i 's/usr/avr-gcc-10.1.0-x64-linux/g' Makefile
make all

cd ..
./convert.sh Sota2
cd Sota2
sed -i 's/usr/avr-gcc-10.1.0-x64-linux/g' Makefile
make all
