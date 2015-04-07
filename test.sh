#!/bin/sh

NUMPROC=$(echo "l($1)/l(2) + 1" | bc -l | cut -d "." -f 1)

mpic++ --prefix /usr/local/share/OpenMPI pms.cpp -o pms

dd if=/dev/urandom bs=1 count=$1 of=numbers 2> /dev/null

mpirun --prefix /usr/local/share/OpenMPI -np $NUMPROC pms    


