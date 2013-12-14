#!/bin/bash
set -e
cd libeo
make -f Makefile.linux ARCH=x64 debug
cd ..
make -f Makefile.linux ARCH=x64 debug
./game_x64.bin 
