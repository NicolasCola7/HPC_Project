#!/bin/bash
# run-mpi-program.sh

#SBATCH -n 4
#SBATCH --time=0-00:05:00
#SBATCH --output slurm-%j.out
#SBATCH --partition=rtx2080

echo "=== Compiling ==="
gcc -O2 -std=c99 -Wall -Wpedantic gen.c -o gen -lm
mpicc -O2 -std=c99 -Wall -Wpedantic mpi-correlogram.c -o mpi-correlogram -lm

echo "=== Running MPI with $SLURM_NTASKS tasks ==="
./gen 2 1000000 data1M
srun --mpi=pmix ./mpi-correlogram data1M_noisy.txt out1M.txt 2048

echo "=== End of Job ==="
