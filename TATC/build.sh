#!/bin/sh
./convert.sh Debug
./convert.sh Release
cd Debug
make all
cat TcvrControl.hex
cd ../Release
make all
cat TcvrControl.hex
