#!/bin/sh
./convert.sh 5Band
cd 5Band
make all

cd ..
./convert.sh Sota2
cd Sota2
make all

cd ..
./convert.sh SOTA5
cd SOTA5
make all
