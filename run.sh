#!/bin/sh
echo "start at $(date '+%d/%m/%Y %H:%M:%S')" >> QueensAttack.csv
echo "core,k,n,time" >> QueensAttack.csv
for ((core = 2; core <= 64; core = core * 2)); do
    for ((k = 3; k <= 8; k++)); do
        for ((n = 3; n <= 10; n++)); do
            # Unix command here #
            mpirun -np $core ./mpi_queensattack $n $k 1 0 1
        done
    done
done
echo "stop at $(date '+%d/%m/%Y %H:%M:%S')" >> QueensAttack.csv