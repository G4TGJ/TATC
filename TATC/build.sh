#!/bin/sh
./convert.sh 5Band
cd 5Band
make all
./convert.sh Sota2
cd Sota2
make all
