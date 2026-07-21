#!/bin/bash

## run-omp-program.sh
##
## This script can be used to submit an OpenMP program to the DISI HPC
## cluster using the Slurm job scheduler.
##
## To submit a job:
##      sbatch run-omp-program.sh
##
## To submit a job specifying the number of threads
## (bypassws the default value specified by --cpus-per-task)
##
##      sbatch -c 2 run-omp-program.sh
##
## To check the queue:
##      squeue
##
## To cancel a job:
##      scancel <jobid>
##
## Last modified in 2026-02-17 by Moreno Marzolla.

#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task 4
#SBATCH --time=0-00:05:00
#SBATCH --output slurm-%j.out
#SBATCH --partition=rtx2080

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

echo "=== Compiling ==="
gcc -std=c99 -Wall -Wpedantic -fopenmp omp-correlogram.c -o omp-correlogram

echo "=== Running OpenMP program with $OMP_NUM_THREADS threads ==="
./omp-correlogram

echo "=== End of Job ==="
