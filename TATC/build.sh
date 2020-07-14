#!/bin/sh
./convert.sh Debug
./convert.sh Release
cd Debug
make all
cd ../Release
make all
