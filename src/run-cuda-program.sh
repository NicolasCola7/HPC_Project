#!/bin/bash

## run-cuda-program.sh
##
## This script can be used to compile and run a CUDA program to the
## DISI HPC cluster using the SLURM job scheduler.
##
## To submit a job:
##      sbatch ./run-cuda-program.sh
##
## To check the queue:
##      squeue
##
## To cancel a job:
##      scancel <jobid>
##
## Last modified on 2026-02-17 by Moreno Marzolla.

#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --gres=gpu:1
#SBATCH --time=0-00:05:00
#SBATCH --output slurm-%j.out

echo "=== Compiling ==="
nvcc cuda-correlogram.cu -o cuda-correlogram

echo "=== CUDA program starts ==="
./cuda-correlogram

echo "=== End of Job ==="
